import { useState } from "react";
import { GradientWave } from "./components/ui/gradient-wave";
import { Navbar } from "./components/Navbar";
import { Hero } from "./components/Hero";
import { AppPreview } from "./components/AppPreview";
import { Features } from "./components/Features";
import { PrivacyModal } from "./components/PrivacyModal";
import { Footer } from "./components/Footer";

export function App() {
  const [privacyOpen, setPrivacyOpen] = useState(false);

  return (
    <div className="relative min-h-screen bg-[#070B09] text-[#F0F5F2] selection:bg-[#1CE47E]/30 selection:text-[#1CE47E] overflow-x-hidden font-sans">
      <div className="fixed inset-0 pointer-events-none z-0 opacity-35">
        <GradientWave
          colors={[
            "#030705",
            "#07170E",
            "#0B2919",
            "#11462A",
            "#1CE47E",
            "#020604",
          ]}
          shadowPower={6}
          noiseSpeed={0.000008}
          deform={{ incline: 0.35, noiseAmp: 220, noiseFlow: 3.5 }}
        />
        <div className="absolute inset-0 bg-[#070B09]/70" />
      </div>

      <div className="relative z-10 flex flex-col min-h-screen">
        <Navbar onOpenPrivacy={() => setPrivacyOpen(true)} />

        <main className="flex-1">
          <Hero />
          <AppPreview />
          <Features />
        </main>

        <Footer onOpenPrivacy={() => setPrivacyOpen(true)} />
      </div>

      <PrivacyModal isOpen={privacyOpen} onClose={() => setPrivacyOpen(false)} />
    </div>
  );
}

export default App;
