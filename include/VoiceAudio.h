#pragma once
#include <Arduino.h>
#include <math.h>

// Center ADC samples into a single contiguous PCM8 buffer before TLS.
// Normal microphone measurements fit within +/-127 ADC counts.
class VoiceAudio {
 public:
  static constexpr unsigned rate=8000, limit=80000;
  uint8_t* data=nullptr;
  int center=1933;
  uint64_t sum=0,squares=0;
  unsigned pcmClipped=0;
  unsigned count=0, late=0, elapsed=0;
  double mean=0,rms=0,gain=0;
  unsigned peak=0,clipped=0;
  ~VoiceAudio(){clear();}
  void clear(){free(data);data=nullptr;}
  bool allocate(){data=(uint8_t*)malloc(limit);return data!=nullptr;}
  uint8_t pcm(unsigned i)const{return data[i];}
  void put(unsigned i,uint16_t v){
   sum+=v;squares+=uint32_t(v)*v;if(v<=10||v>=4085)clipped++;
   int sample=int(v)-center;if(sample<-127||sample>127)pcmClipped++;
   data[i]=constrain(sample+128,0,255);
  }
  void measure(){
   mean=double(sum)/count;rms=sqrt(max(0.0,double(squares)/count-mean*mean));
   for(unsigned i=0;i<count;i++)peak=max(peak,(unsigned)ceil(abs(double(data[i])-128-(mean-center))));
  }
  void pack(){
   gain=peak?min(64.0,114.0/peak):1;
   for(unsigned i=0;i<count;i++)data[i]=(uint8_t)constrain((int)lround((double(data[i])-128-(mean-center))*gain)+128,0,255);
  }
};

// Multipart request streamed directly to HTTPClient, without a second WAV copy.
class VoiceMultipart: public Stream {
 String head,tail;
 const VoiceAudio& audio;
 uint8_t wav[44]={0};
 size_t cursor=0,n;
 void le(unsigned pos,uint32_t value,unsigned bytes){for(unsigned j=0;j<bytes;j++)wav[pos+j]=(value>>(8*j))&255;}
 public:
 VoiceMultipart(const VoiceAudio& data,unsigned sampleRate):audio(data),n(data.count){
  head="--teraVoiceBoundary\r\nContent-Disposition: form-data; name=\"model\"\r\n\r\nwhisper-large-v3-turbo\r\n--teraVoiceBoundary\r\nContent-Disposition: form-data; name=\"language\"\r\n\r\npt\r\n--teraVoiceBoundary\r\nContent-Disposition: form-data; name=\"response_format\"\r\n\r\nverbose_json\r\n--teraVoiceBoundary\r\nContent-Disposition: form-data; name=\"temperature\"\r\n\r\n0\r\n--teraVoiceBoundary\r\nContent-Disposition: form-data; name=\"file\"; filename=\"voice.wav\"\r\nContent-Type: audio/wav\r\n\r\n";
  tail="\r\n--teraVoiceBoundary--\r\n";
  memcpy(wav,"RIFF",4);le(4,36+n,4);memcpy(wav+8,"WAVEfmt ",8);le(16,16,4);le(20,1,2);le(22,1,2);le(24,sampleRate,4);le(28,sampleRate,4);le(32,1,2);le(34,8,2);memcpy(wav+36,"data",4);le(40,n,4);
 }
 size_t size()const{return head.length()+44+n+tail.length();}
 int available()override{return size()-cursor;}
 int peek()override{
  size_t p=cursor;if(p<head.length())return (uint8_t)head[p];p-=head.length();
  if(p<44)return wav[p];p-=44;if(p<n)return audio.pcm(p);p-=n;
  return p<tail.length()?(uint8_t)tail[p]:-1;
 }
 int read()override{int value=peek();if(value>=0)cursor++;return value;}
 size_t readBytes(char* buffer,size_t length)override{
  size_t count=min(length,size()-cursor);
  for(size_t i=0;i<count;i++)buffer[i]=read();
  // HTTPClient yields with delay(0), which may starve the idle watchdog task
  // during a long TLS upload. Block briefly once per upload block instead.
  delay(1);return count;
 }
 size_t readBytes(uint8_t* buffer,size_t length)override{return readBytes((char*)buffer,length);}
 void flush()override{}
 size_t write(uint8_t)override{return 0;}
};
