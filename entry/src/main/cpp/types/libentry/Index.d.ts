export const add: (a: number, b: number) => number;
// index.d.ts  HarmonyOS / OpenHarmony 原生模块声明
export const logInit: (logFile: string, logLevel: number) => void;
export const getVulkanApplication: (
  windowHandle: Object,
  width: number,
  height: number,
  vulkanPath: string
) => number | null;
export const drawFrame: (appHandle: number) => number;
export const destroyVulkanApplication: (appHandle: number) => void;