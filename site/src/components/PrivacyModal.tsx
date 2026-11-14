import { X, ShieldCheck } from "lucide-react";

interface PrivacyModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export function PrivacyModal({ isOpen, onClose }: PrivacyModalProps) {
  if (!isOpen) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 sm:p-6 bg-black/75 font-sans"
      onClick={onClose}
    >
      <div
        className="relative w-full max-w-2xl max-h-[85vh] rounded-lg border border-[#1E3327] bg-[#090F0C] text-[#D3E2DA] flex flex-col overflow-hidden"
        onClick={(e) => e.stopPropagation()}
      >
        <div className="px-6 py-4 border-b border-[#18261E] flex items-center justify-between bg-[#070C0A]">
          <div className="flex items-center gap-2.5">
            <ShieldCheck className="w-4 h-4 text-[#1CE47E]" />
            <h3 className="font-display font-semibold text-[#F0F5F2] text-sm sm:text-base">
              Политика конфиденциальности и условия использования
            </h3>
          </div>
          <button
            type="button"
            onClick={onClose}
            className="w-8 h-8 rounded-lg border border-[#18261E] bg-[#0D1712] hover:border-[#294536] text-[#8EA498] hover:text-[#F0F5F2] flex items-center justify-center transition-colors duration-150 cursor-pointer"
            aria-label="Закрыть"
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        <div className="p-6 overflow-y-auto space-y-5 text-xs sm:text-sm leading-relaxed text-[#8EA498]">
          <div>
            <h4 className="font-medium text-[#F0F5F2] mb-1">
              1. Общие положения и статус проекта
            </h4>
            <p>
              Программное обеспечение <strong className="text-[#D3E2DA]">«NodeHub»</strong> является независимым бесплатным фанатским клиентом, созданным для поиска, каталогизации и локальной организации модификаций к игре BeamNG.drive.
            </p>
          </div>

          <div className="p-4 rounded-lg bg-[#0D1913] border border-[#1E3327] text-[#D2E4DA]">
            <h4 className="font-medium text-[#1CE47E] mb-1">
              2. Отказ от аффилированности (STMods и BeamNG GmbH)
            </h4>
            <p className="text-xs leading-relaxed">
              Приложение «NodeHub» не является официальным продуктом BeamNG GmbH и не связано, не аффилировано, не одобрено и не поддерживается администрацией сайта stmods.org. Товарные знаки BeamNG.drive принадлежат BeamNG GmbH. Все права на материалы сайта STMods принадлежат их владельцам.
            </p>
          </div>

          <div>
            <h4 className="font-medium text-[#F0F5F2] mb-1">
              3. Авторские права на модификации
            </h4>
            <p>
              Все модификации, 3D-модели, текстуры, скриншоты и описания принадлежат их авторам. NodeHub сохраняет имена создателей и исходные описания в карточках модов без изменений.
            </p>
          </div>

          <div>
            <h4 className="font-medium text-[#F0F5F2] mb-1">
              4. Хостинг и источники загрузки
            </h4>
            <p>
              NodeHub не размещает и не хранит архивы модов на собственных серверах. Скачивание выполняется напрямую с публичных страниц и файлообменников (STMods, ModsFire).
            </p>
          </div>

          <div>
            <h4 className="font-medium text-[#F0F5F2] mb-1">
              5. Отсутствие сбора персональных данных
            </h4>
            <p>
              NodeHub не собирает и не передаёт персональные данные пользователя, телеметрию, учётные записи или IP-адреса. Настройки пути к игре и локальный реестр установленных модов хранятся только на вашем ПК в{" "}
              <code className="text-[#D3E2DA]">%APPDATA%\NodeHub</code>.
            </p>
          </div>

          <div>
            <h4 className="font-medium text-[#F0F5F2] mb-1">
              6. Ограничение ответственности
            </h4>
            <p>
              Программа предоставляется на условиях «КАК ЕСТЬ» (AS IS). Пользователь самостоятельно отвечает за совместимость модификаций и резервные копии файлов.
            </p>
          </div>
        </div>

        <div className="px-6 py-3.5 border-t border-[#18261E] bg-[#070C0A] flex items-center justify-between">
          <span className="text-xs text-[#6E8579]">NodeHub v1.0.0</span>
          <button
            type="button"
            onClick={onClose}
            className="px-4 py-1.5 rounded-lg bg-[#1CE47E] text-[#041A0E] font-semibold text-xs hover:bg-[#32EA8D] transition-colors duration-150 cursor-pointer"
          >
            Закрыть
          </button>
        </div>
      </div>
    </div>
  );
}
