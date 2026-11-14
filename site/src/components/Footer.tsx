interface FooterProps {
  onOpenPrivacy: () => void;
}

export function Footer({ onOpenPrivacy }: FooterProps) {
  return (
    <footer className="border-t border-[#18261E] bg-[#060A08] py-10 px-6 text-xs text-[#6E8579] font-sans">
      <div className="max-w-5xl mx-auto flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
        <div>
          <span className="font-display font-semibold text-xs text-[#D4E0D9]">NodeHub</span>
          <span className="mx-2 text-[#293E32]">·</span>
          <span>
            Независимый клиент для BeamNG.drive. Не связан с BeamNG GmbH и stmods.org.
          </span>
        </div>

        <button
          type="button"
          onClick={onOpenPrivacy}
          className="text-[#8EA498] hover:text-[#F0F5F2] transition-colors duration-150 cursor-pointer shrink-0"
        >
          Политика конфиденциальности
        </button>
      </div>
    </footer>
  );
}
