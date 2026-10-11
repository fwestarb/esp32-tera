(() => {
  const en = {
    skip: 'Skip to content', navInstall: 'Install', navUse: 'Use', navFaq: 'Help',
    title: 'A small board.<br>One conversation<br><em>at a time.</em>',
    lead: 'Meet Tera: an AI assistant on your ESP32. Speak into the microphone, read on the display and, with your computer connected, hear the replies.',
    download: 'Download Alpha 1.0', source: 'View source', downloadHint: 'Windows + Python 3.12 · open source · your own API key',
    screenText: 'Hello. Where shall we begin?', illustration: 'Project illustration · demonstration animation', microphone: 'MICROPHONE',
    featuresTag: '01 / MEET TERA', featuresTitle: 'Small on the outside.<br>Connected on the inside.',
    featuresIntro: 'The board handles the interface. AI and transcription run in the cloud; speech output is generated on your computer.',
    f1Title: 'Hold. Speak. Release.', f1Text: 'Record up to ten seconds with GPIO35. Groq transcribes your speech and Tera shows the reply on the display.',
    f2Title: 'A companion on your PC', f2Text: 'Hear the female Dora voice and ask to open Calculator, Notepad, File Explorer, your browser, GitHub or YouTube.',
    f3Title: 'Configure it your way', f3Text: 'Choose Wi-Fi, API and model over USB or from your phone on Tera’s temporary network. Your key stays on the board.',
    photoAlt: 'Real TTGO T-Display prototype connected to a discrete microphone amplifier on a breadboard',
    prototypeTag: 'FROM THE WORKBENCH TO A CONVERSATION', prototypeTitle: 'A real project.<br>In development.',
    prototypeText: 'This is the development prototype: an original TTGO T-Display and a microphone with a discrete amplifier. A demonstration of electronics, AI and software working together.',
    hardwareLink: 'View assembly and wiring', photoHint: 'Prototype photograph taken during an earlier microphone test.',
    installTag: '02 / START HERE', installTitle: 'From installation<br>to your first question.',
    installIntro: 'Choose a folder on your computer. Installation downloads the dependencies and voice model; afterwards, speech runs locally.',
    requirementsTitle: 'Before you start', r1: 'Original TTGO T-Display · ESP32 · 4 MB', r2: 'USB data cable', r3: '2.4 GHz Wi-Fi and internet access',
    r4: 'Windows and Python 3.12 on PATH', r5: 'Your own Groq API key on its free plan', r6: 'For speech input: amplified microphone on GPIO32',
    boardWarning: 'This firmware is specific to the original TTGO. The T-Display S3 uses different hardware.',
    pythonLink: 'Python 3.12 ↗', driverLink: 'CP210x USB driver ↗', keyLink: 'Create a Groq key ↗',
    s1Title: 'Download and extract', s1Text: 'Download Alpha 1.0 and extract the ZIP into your own folder. Install Python 3.12 with the option to add it to PATH enabled.', downloadZip: 'Download ZIP package ↗',
    s2Title: 'Prepare the application', s2Text: 'Run instalar.cmd. It installs the Python environments and Dora voice. The initial voice download is about 338 MB, plus the libraries.', copy: 'Copy',
    s3Title: 'Flash the TTGO', s3Text: 'Connect the board, close applications using its serial port and run gravar-firmware.cmd. Enter your port, for example COM3. This replaces the current program and preserves NVS configuration.',
    s4Title: 'Configure Tera', s4Text: 'Open configurar.cmd. Enter your 2.4 GHz Wi-Fi, Groq key and model, then save to the board. Wait for Wi-Fi and clock synchronization before asking your first question.',
    installNote: 'The package contains scripts and firmware; Python is required. Each person uses their own key. Free APIs have quotas and may be unavailable.', fullGuide: 'Full guide ↗',
    useTag: '03 / EVERYDAY USE', useTitle: 'A simple conversation.<br>A few gestures are enough.',
    u1Title: 'On the board', u1Text: 'Hold GPIO35, speak and release to send. Record between half a second and ten seconds. Tap GPIO0 to turn response pages.', talk: 'speak', page: 'page',
    u2Title: 'From your phone', u2Text: 'With the board powered on, hold GPIO0 for two seconds. Join the TERA-… network using the password on the display and open http://192.168.4.1. Save to return to your Wi-Fi. The portal closes after ten minutes.',
    bootHint: 'Do not hold GPIO0 during reset: it also activates flashing mode.',
    u3Title: 'With your computer', u3Text: 'Keep the application running to hear replies and open programs. X minimizes it; Encerrar assistente quits. Select a voice and click Ouvir exemplo to preview it. Parar voz stops playback.', commandExample: '“Tera, pode abrir a calculadora?”',
    u4Title: 'Change the model', u4Text: 'Choose GPT-OSS 120B or 20B and use Trocar somente o modelo. Your choice is saved without reflashing. OpenRouter supports free text models; speech input requires Groq in this alpha.',
    faqTag: '04 / DOCUMENTATION', faqTitle: 'Good to know.', faqIntro: 'An alpha with real features and documented limits. Here are answers to the most common questions.', testsLink: 'View test results ↗',
    q1: 'Does it work without a computer?', a1: 'The powered board can capture speech and display replies over Wi-Fi without a PC. Speech output and opening applications require Windows with the companion app running. No physical speaker is installed.',
    q2: 'Is the AI free?', a2: 'Use your own Groq account on its free plan or free OpenRouter models. Quotas and capacity limits apply. Tera does not change your account billing. Check your plan before using it.',
    q3: 'Where is my data stored?', a3: 'Your key is stored in the board’s unencrypted NVS. Audio goes to Groq; questions and one previous exchange go to the chat provider. Speech output is generated locally on the PC. Do not share flash dumps.',
    q4: 'I got a 401, 429 or model error', a4: '401 means an invalid key or denied access. 429 means a quota or capacity limit: wait and check your limits. If a model is rejected, choose a compatible one. Also check 2.4 GHz Wi-Fi and clock synchronization.',
    q5: 'USB connection or speech is not working', a5: 'Use a data cable and the CP210x driver. Close other serial monitors and reconnect the board if needed. For speech, check Dora installation, the Falar respostas no PC option and Windows audio output.',
    q6: 'What are Alpha 1.0’s limitations?', a6: 'No physical keyboard, onboard speaker or streaming. Recording is limited to ten seconds, with short conversation memory. Commands use a defined application list. Alpha testing did not include a physical phone connection or a new user speech recording.',
    closingTag: 'OPEN TO EXPLORE', closingTitle: 'Build. Talk.<br>Let it grow.', closingText: 'Source code, wiring and test results are available to study, experiment with and contribute to.',
    releaseLink: 'Release files and SHA-256 ↗', footer: 'A project by fwestarb · Alpha 1.0 · MIT source code', licenses: 'Licenses ↗'
  };
  const pt = {};
  document.querySelectorAll('[data-i18n]').forEach(el => { pt[el.dataset.i18n] = el.innerHTML; });
  let language = 'pt';
  let toastTimer;
  function setLanguage(next) {
    language = next === 'en' ? 'en' : 'pt';
    const dictionary = language === 'en' ? en : pt;
    document.documentElement.lang = language === 'en' ? 'en' : 'pt-BR';
    document.querySelectorAll('[data-i18n]').forEach(el => {
      if (dictionary[el.dataset.i18n] !== undefined) el.innerHTML = dictionary[el.dataset.i18n];
    });
    document.querySelector('[data-alt="photoAlt"]').alt = language === 'en' ? en.photoAlt : 'Protótipo real da TTGO T-Display ligado ao amplificador do microfone em uma protoboard';
    document.querySelectorAll('[data-lang]').forEach(el => el.setAttribute('aria-pressed', String(el.dataset.lang === language)));
    document.querySelectorAll('[data-copy]').forEach(el => el.setAttribute('aria-label', language === 'en' ? 'Copy command' : 'Copiar comando'));
    document.title = language === 'en' ? 'esp32-tera — a small board, a conversation' : 'esp32-tera — uma pequena placa, uma conversa';
    try { localStorage.setItem('tera-language', language); } catch (_) { /* Storage is optional. */ }
  }
  document.querySelectorAll('[data-lang]').forEach(el => el.addEventListener('click', () => setLanguage(el.dataset.lang)));
  try { setLanguage(localStorage.getItem('tera-language') || 'pt'); } catch (_) { setLanguage('pt'); }
  document.querySelectorAll('[data-copy]').forEach(el => el.addEventListener('click', async () => {
    const toast = document.getElementById('toast');
    try {
      await navigator.clipboard.writeText(el.dataset.copy);
      toast.textContent = language === 'en' ? 'Command copied' : 'Comando copiado';
    } catch (_) {
      toast.textContent = language === 'en' ? 'Select the command and copy it manually.' : 'Selecione o comando e copie manualmente.';
    }
    clearTimeout(toastTimer); toast.classList.add('show');
    toastTimer = setTimeout(() => toast.classList.remove('show'), 3000);
  }));
  if ('IntersectionObserver' in window && !matchMedia('(prefers-reduced-motion: reduce)').matches) {
    const observer = new IntersectionObserver(entries => entries.forEach(entry => {
      if (entry.isIntersecting) { entry.target.classList.add('visible'); observer.unobserve(entry.target); }
    }), { threshold: 0.08 });
    document.documentElement.classList.add('js');
    document.querySelectorAll('.reveal').forEach(el => observer.observe(el));
  }
})();
