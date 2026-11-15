import { Download } from "lucide-react";

export function Hero() {
  return (
    <section className="pt-36 pb-16 md:pt-44 md:pb-20 px-6 max-w-5xl mx-auto text-center">
      <h1 className="font-display text-3xl sm:text-4xl md:text-[42px] font-bold tracking-tight text-[#F0F5F2] leading-[1.2] mb-5">
        Моды для BeamNG.drive
      </h1>

      <p className="font-sans text-base sm:text-lg text-[#8EA498] max-w-2xl mx-auto leading-relaxed mb-9">
        Настольный лаунчер для каталога STMods. Скачивает архивы напрямую, раскладывает
        машины и карты по нужным папкам и запускает игру без браузера.
      </p>

      <div className="flex flex-col items-center gap-3">
        <a
          href="https://github.com/lidamargin-coder/NodeHub/releases/latest/download/NodeHub_Setup_v1.1.0.exe"
          download
          className="font-display inline-flex items-center justify-center gap-2.5 px-7 py-3.5 rounded-lg bg-[#1CE47E] text-[#041A0E] font-bold text-sm hover:bg-[#32EA8D] transition-colors duration-150"
        >
          <Download className="w-4 h-4 stroke-[2.25]" />
          <span>Скачать для Windows</span>
        </a>

        <p className="font-sans text-xs text-[#6E8579] tabular-nums">
          v1.1.0 · 64-bit · 25 МБ
        </p>
      </div>
    </section>
  );
}
