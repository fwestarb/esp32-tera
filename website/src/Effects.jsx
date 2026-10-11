import { useState } from 'react';
import HalftoneReveal from './components/HalftoneReveal';
import SideRays from './components/SideRays';
import FlexCarousel from './components/FlexCarousel';
export function TreeEffect({ src }) {
  const [ready, setReady] = useState(false);
  return <div className="effect-surface" data-ready={ready}><HalftoneReveal src={src} onReady={()=>setReady(true)} fit="contain" mode="color" paperColor="#121512" inkColor="#b9d7a3" cellSize={4} reverse revealRadius={75} linger={0.6} intro={false} clickBurst={false} /></div>;
}
export function Rays({ origin = 'top-right' }) {
  return <SideRays speed={0.35} rayColor1="#728465" rayColor2="#b9d7a3" intensity={0.6} spread={1.7} origin={origin} opacity={0.22} saturation={0.6} />;
}
export function Gallery({ items, label }) {
  return <FlexCarousel items={items} preset="arch" intro="rise" cardHeight={0.58} gap={30} squeeze={0.06} dispersion={0} bend={0.12} focusOnClick captions captureWheel={false} ariaLabel={label} />;
}
