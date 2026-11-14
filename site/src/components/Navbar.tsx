import { useState, useEffect } from "react";

interface NavbarProps {
  onOpenPrivacy: () => void;
}

export function Navbar({ onOpenPrivacy }: NavbarProps) {
  const [scrolled, setScrolled] = useState(false);

  useEffect(() => {
    const handleScroll = () => {
      setScrolled(window.scrollY > 16);
    };
    window.addEventListener("scroll", handleScroll);
    return () => window.removeEventListener("scroll", handleScroll);
  }, []);

  return (
    <header
      className={`fixed top-0 left-0 right-0 z-50 transition-colors duration-150 border-b ${
        scrolled
          ? "bg-[#070B09]/95 border-[#18261E] py-3.5"
          : "bg-[#070B09]/80 border-[#18261E]/50 py-4"
      }`}
    >
      <div className="max-w-5xl mx-auto px-6 flex items-center justify-between">
        <a href="#" className="flex items-center gap-2.5 text-[#F0F5F2] hover:text-[#1CE47E] transition-colors duration-150">
          <img src="./logo.svg" alt="NodeHub" className="w-6 h-6 object-contain" />
          <span className="font-display font-bold text-sm tracking-tight">NodeHub</span>
        </a>

        <nav className="font-sans flex items-center gap-6 sm:gap-8 text-sm text-[#8EA498]">
          <a href="#preview" className="hover:text-[#F0F5F2] transition-colors duration-150">
            Скриншоты
          </a>
          <a href="#features" className="hover:text-[#F0F5F2] transition-colors duration-150">
            Возможности
          </a>
          <button
            type="button"
            onClick={onOpenPrivacy}
            className="hover:text-[#F0F5F2] transition-colors duration-150 cursor-pointer"
          >
            Конфиденциальность
          </button>
          <a
            href="https://github.com/lidamargin-coder/NodeHub"
            target="_blank"
            rel="noreferrer"
            className="text-[#8EA498] hover:text-[#F0F5F2] transition-colors duration-150 hidden sm:inline-flex"
            aria-label="GitHub Repository"
            title="GitHub Repository"
          >
            <svg className="w-4 h-4 fill-current" viewBox="0 0 24 24">
              <path d="M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0024 12c0-6.63-5.37-12-12-12z" />
            </svg>
          </a>
        </nav>
      </div>
    </header>
  );
}
