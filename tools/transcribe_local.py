"""Local transcription. Downloads model weights; never uploads the audio."""
import argparse
import json
from pathlib import Path
from faster_whisper import WhisperModel

parser=argparse.ArgumentParser()
parser.add_argument('audio')
parser.add_argument('--model',default='base')
args=parser.parse_args()
source=Path(args.audio)
if not source.is_file():raise FileNotFoundError(source)
print('Carregando Whisper local...',flush=True)
model=WhisperModel(args.model,device='cpu',compute_type='int8',cpu_threads=4,
                  download_root='.venv-transcription/models')
segments,info=model.transcribe(str(source),language='pt',beam_size=5,
                              temperature=0,condition_on_previous_text=False,
                              vad_filter=False)
rows=[]
for segment in segments:
    rows.append({'start':segment.start,'end':segment.end,'text':segment.text.strip(),
                 'avg_logprob':segment.avg_logprob,'no_speech_prob':segment.no_speech_prob})
text=' '.join(row['text'] for row in rows)
report={'model':args.model,'local':True,'audio':str(source.resolve()),
        'duration':info.duration,'language':'pt','text':text,'segments':rows,
        'note':'Automatic hypothesis; not a human-verified transcript.'}
source.with_suffix('.transcricao.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
source.with_suffix('.transcricao.txt').write_text(text+'\n',encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2),flush=True)
