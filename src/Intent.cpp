#include "Intent.h"

String classifyAction(String text) {
 const char* accents[]={"á","à","ã","â","é","ê","í","ó","ô","õ","ú","ç","Á","À","Ã","Â","É","Ê","Í","Ó","Ô","Õ","Ú","Ç"};
 const char* ascii[]={"a","a","a","a","e","e","i","o","o","o","u","c","a","a","a","a","e","e","i","o","o","o","u","c"};
 for(unsigned i=0;i<24;i++)text.replace(accents[i],ascii[i]);
 text.toLowerCase();
 // Quoted, negative, hypothetical and instructional sentences are not actions.
 if(text.indexOf('"')>=0||text.indexOf('\'')>=0||text.indexOf("“")>=0||text.indexOf("”")>=0)return "";
 text.replace(","," ");text.replace("?"," ");text.replace("!"," ");text.replace("."," ");
 text.trim();while(text.indexOf("  ")>=0)text.replace("  "," ");
 String padded=" "+text+" ";
 const char* blocked[]={"nao","nunca","jamais","evite","sem","se","como","quando","talvez","exemplo"};
 for(auto word:blocked)if(padded.indexOf(" "+String(word)+" ")>=0)return "";
 text.replace(","," ");text.replace("?"," ");text.replace("!"," ");text.replace("."," ");
 text.trim();while(text.indexOf("  ")>=0)text.replace("  "," ");
 if(text.startsWith("tera "))text.remove(0,5);
 if(text.startsWith("por favor "))text.remove(0,10);
 const char* prefixes[]={"voce poderia abrir ","voce pode abrir ","poderia abrir ","pode abrir ","consegue abrir ","quero que voce abra ","quero abrir ","gostaria de abrir ","abra ","abre ","abrir ","inicie ","iniciar "};
 bool request=false;for(auto prefix:prefixes)if(text.startsWith(prefix)){text.remove(0,strlen(prefix));request=true;break;}
 if(!request)return "";
 const char* suffixes[]={" por favor"," para mim"," pra mim"," agora"};
 bool again=true;while(again){again=false;for(auto suffix:suffixes)if(text.endsWith(suffix)){text.remove(text.length()-strlen(suffix));again=true;}}
 if(text.startsWith("o ")||text.startsWith("a "))text.remove(0,2);
 if(text=="calculadora"||text=="calcula dora"||text=="calc")return "calculator";
 if(text=="bloco de notas"||text=="notepad")return "notepad";
 if(text=="explorador"||text=="explorador de arquivos"||text=="explorer")return "explorer";
 if(text=="navegador"||text=="browser")return "browser";
 if(text=="github"||text=="git hub")return "github";
 if(text=="youtube"||text=="you tube")return "youtube";
 return "clarify";
}
