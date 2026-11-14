import { useState, useEffect } from "react";

interface MediaTab {
  id: string;
  label: string;
  type: "image" | "video";
  src: string;
}

const SCREEN_TABS: MediaTab[] = [
  {
    id: "catalog",
    label: "Каталог и поиск",
    type: "image",
    src: "./media/screen_catalog.png",
  },
  {
    id: "installed",
    label: "Установленные моды",
    type: "image",
    src: "./media/screen_installed.png",
  },
  {
    id: "maps",
    label: "Карты и трассы",
    type: "image",
    src: "./media/screen_maps.png",
  },
];

export function AppPreview() {
  const [hasVideo, setHasVideo] = useState(false);
  const [activeId, setActiveId] = useState<string>("catalog");

  useEffect(() => {
    const checkVideo = async () => {
      try {
        const res = await fetch("./media/hero_video.mp4", { method: "HEAD" });
        if (res.ok && res.headers.get("content-type")?.includes("video")) {
          setHasVideo(true);
          setActiveId("video");
        }
      } catch {
        // hero_video.mp4 is optional
      }
    };
    checkVideo();
  }, []);

  const tabs: MediaTab[] = hasVideo
    ? [
        {
          id: "video",
          label: "Видеообзор",
          type: "video",
          src: "./media/hero_video.mp4",
        },
        ...SCREEN_TABS,
      ]
    : SCREEN_TABS;

  return (
    <section id="preview" className="max-w-5xl mx-auto px-6 pb-24 md:pb-32">
      {/* Switcher Navigation */}
      <div className="flex flex-wrap items-center justify-center gap-2 mb-6">
        {tabs.map((tab) => {
          const isActive = tab.id === activeId;
          return (
            <button
              key={tab.id}
              type="button"
              onClick={() => setActiveId(tab.id)}
              className={`px-4 py-2 rounded-lg text-xs font-medium border transition-colors duration-200 cursor-pointer focus:outline-none ${
                isActive
                  ? "bg-[#13281D] border-[#1CE47E]/60 text-[#F0F5F2]"
                  : "bg-[#090F0C] border-[#18261E] text-[#8EA498] hover:text-[#F0F5F2] hover:border-[#24392D]"
              }`}
            >
              {tab.label}
            </button>
          );
        })}
      </div>

      {/* Pre-rendered media frame: all screenshots stay mounted in DOM for instant, zero-flicker crossfading */}
      <div className="rounded-lg border border-[#18261E] bg-[#090F0C] p-0.5">
        <div className="relative w-full aspect-[1240/760] rounded-[6px] overflow-hidden bg-[#090F0C]">
          {tabs.map((tab) => {
            const isActive = tab.id === activeId;
            return (
              <div
                key={tab.id}
                className={`absolute inset-0 w-full h-full transition-opacity duration-300 ease-in-out ${
                  isActive
                    ? "opacity-100 z-10 pointer-events-auto"
                    : "opacity-0 z-0 pointer-events-none"
                }`}
              >
                {tab.type === "video" ? (
                  <video
                    src={tab.src}
                    autoPlay
                    loop
                    muted
                    playsInline
                    controls
                    className="w-full h-full object-cover block"
                  />
                ) : (
                  <img
                    src={tab.src}
                    alt={`NodeHub — ${tab.label}`}
                    loading="eager"
                    decoding="sync"
                    className="w-full h-full object-cover block select-none"
                  />
                )}
              </div>
            );
          })}
        </div>
      </div>
    </section>
  );
}
