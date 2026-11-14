import { DownloadCloud, FolderCheck, Layers, Play } from "lucide-react";

const ITEMS = [
  {
    icon: DownloadCloud,
    title: "Прямое скачивание без браузера",
    desc: "NodeHub сам проходит страницу загрузки ModsFire и загружает архивы в очереди до 5 модов одновременно.",
  },
  {
    icon: FolderCheck,
    title: "Автосортировка по папкам игры",
    desc: "Карты помещаются в content/levels, а автомобили и техника — в content/vehicles без ручной распаковки.",
  },
  {
    icon: Layers,
    title: "Менеджер установленных файлов",
    desc: "Сканирует локальную папку BeamNG.drive и позволяет включать или отключать моды (.zip.disabled) без удаления.",
  },
  {
    icon: Play,
    title: "Запуск BeamNG.drive из окна",
    desc: "Автоматически находит 64-битный исполняемый файл игры и запускает её сразу после обновления модов.",
  },
];

export function Features() {
  return (
    <section id="features" className="max-w-5xl mx-auto px-6 pb-28">
      <div className="mb-10">
        <h2 className="font-display text-2xl sm:text-3xl font-bold tracking-tight text-[#F0F5F2] mb-2">
          Возможности лаунчера
        </h2>
        <p className="font-sans text-sm text-[#8EA498]">
          Только основные инструменты для работы с модификациями.
        </p>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-10">
        {ITEMS.map((item) => {
          const Icon = item.icon;
          return (
            <div key={item.title} className="border-t border-[#18261E] pt-5">
              <div className="flex items-center gap-2.5 mb-2">
                <Icon className="w-4 h-4 text-[#1CE47E] shrink-0" />
                <h3 className="font-display text-sm sm:text-base font-semibold text-[#F0F5F2]">
                  {item.title}
                </h3>
              </div>
              <p className="font-sans text-sm text-[#8EA498] leading-relaxed">
                {item.desc}
              </p>
            </div>
          );
        })}
      </div>
    </section>
  );
}
