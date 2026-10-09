"""Record GPIO32 ADC samples over USB; preserve raw data and produce WAV files."""
import argparse
import array
import json
import math
from pathlib import Path
import sys
import time
import wave
import zlib
import serial

def save_wav(path, samples, rate):
    values=array.array('h',samples)
    if sys.byteorder!='little':values.byteswap()
    with wave.open(str(path),'wb') as out:
        out.setnchannels(1);out.setsampwidth(2);out.setframerate(rate);out.writeframes(values.tobytes())

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--silence',action='store_true',help='Capture background noise; instruct the user to stay silent.')
    parser.add_argument('--output',default='recordings/microfone-2026-10-09')
    args=parser.parse_args()
    prefix=Path(args.output);prefix.parent.mkdir(parents=True,exist_ok=True)
    with serial.Serial(args.port,115200,timeout=.5) as conn:
        conn.dtr=False;conn.rts=False
        ready=False;deadline=time.monotonic()+30
        while time.monotonic()<deadline:
            line=conn.readline()
            if line.startswith(b'MIC_TEST') or line.count(b',')==4:ready=True;break
        if not ready:raise RuntimeError('Firmware de microfone nao respondeu.')
        conn.reset_input_buffer();conn.write(b'r')
        instruction='fique em silencio' if args.silence else 'fale'
        print(f'Contagem de 3 segundos na TTGO; {instruction} durante GRAVANDO (10 segundos).',flush=True)
        deadline=time.monotonic()+30
        while time.monotonic()<deadline:
            header=conn.readline()
            if header.startswith(b'AUDIO1 '):break
        else:raise RuntimeError('Cabecalho de audio nao recebido.')
        _,count,elapsed,late,expected_crc=header.decode('ascii').split()
        count,elapsed,late,expected_crc=map(int,(count,elapsed,late,expected_crc))
        if not 1<=count<=80000 or elapsed<=0:raise RuntimeError('Cabecalho invalido.')
        raw=bytearray();deadline=time.monotonic()+40
        while len(raw)<count*2 and time.monotonic()<deadline:
            raw.extend(conn.read(count*2-len(raw)))
        if len(raw)!=count*2:raise RuntimeError('Transferencia incompleta.')
        if zlib.crc32(raw)!=expected_crc:raise RuntimeError('CRC incorreto: audio descartado.')
    values=array.array('H');values.frombytes(raw)
    if sys.byteorder!='little':values.byteswap()
    mean=sum(values)/count
    centered=[v-mean for v in values]
    peak=max(abs(v) for v in centered)
    rms=math.sqrt(sum(v*v for v in centered)/count)
    # Preserve ADC dynamic range; provide a second explicitly amplified listening copy.
    original=[max(-32768,min(32767,round(v*16))) for v in centered]
    gain=min(64,28000/max(1,peak*16))
    listen=[max(-32768,min(32767,round(v*16*gain))) for v in centered]
    rate=round(count*1_000_000/elapsed)
    if not 4000<=rate<=12000:raise RuntimeError('Taxa medida inesperada; verificar amostragem.')
    prefix.with_suffix('.adc').write_bytes(raw)
    save_wav(prefix.with_suffix('.wav'),original,rate)
    listening=prefix.parent/(prefix.name+'-escutar.wav')
    save_wav(listening,listen,rate)
    report={'samples':count,'elapsed_us':elapsed,'measured_sample_rate':rate,'late_samples_over_30us':late,'crc_verified':True,'adc_mean':mean,'adc_rms':rms,'adc_peak_to_peak':max(values)-min(values),'clipped_adc_samples':sum(v<=10 or v>=4085 for v in values),'listening_gain':gain,'original_wav':str(prefix.with_suffix('.wav').resolve()),'listening_wav':str(listening.resolve()),'note':'DC removed; listening copy digitally amplified. No denoising or speech recognition. Rate uses measured duration; jitter is not corrected.'}
    report['capture_instruction']='silence' if args.silence else 'speech'
    prefix.with_suffix('.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report,indent=2),flush=True)

if __name__=='__main__':main()
