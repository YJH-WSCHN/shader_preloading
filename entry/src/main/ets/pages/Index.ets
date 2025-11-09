import vk from 'libentry.so';
import { hilog } from '@kit.PerformanceAnalysisKit';
import fs from '@ohos.fileio';

@Entry
@Component
struct Index {
  @State private msg: string = '点击画三角形';
  private xComponentController: XComponentController = new XComponentController();
  private surfaceId: string = '';
  private appHandle: number | null = null;
  private timer: number = -1;

  aboutToDisappear(): void {
    this.stopDraw();
  }

  build() {
    Column({ space: 20 }) {
      Text(this.msg).fontSize(24).margin(10)

      XComponent({
        id: 'vk_surface',
        type: XComponentType.SURFACE,
        controller: this.xComponentController
      })
        .onLoad(() => this.onSurfaceReady())
        .onDestroy(() => this.stopDraw())
        .width('100%')
        .height('70%')

      Button('画出三角形')
        .onClick(() => this.startDraw())
        .fontSize(20)
        .width(200)
        .height(50)
    }
    .width('100%')
    .height('100%')
    .padding(10)
  }

  private onSurfaceReady(): void {
    this.surfaceId = this.xComponentController.getXComponentSurfaceId();
    hilog.info(0x0000, 'VK', `Surface ready id=${this.surfaceId}`);
  }

  private startDraw(): void {
    if (this.appHandle !== null) return;
    if (!this.surfaceId) {
      this.msg = 'surface 未就绪';
      return;
    }

    const logPath = getContext().filesDir + '/vk.log';
    console.info('filesDir 绝对路径 = ' + logPath);

    const rect = this.xComponentController.getXComponentSurfaceRect();
    const w = rect.surfaceWidth;
    const h = rect.surfaceHeight;

    this.appHandle = vk.getVulkanApplication(
      BigInt(this.surfaceId),
      w,
      h,
      ""
    );
    if (this.appHandle === null) {
      this.msg = 'Vulkan 创建失败';
      return;
    }

    this.msg = '正在画三角形...';
    this.loopDraw();
  }

  private loopDraw(): void {
    let frame = 0;
    this.timer = setInterval(() => {
      const st = vk.drawFrame(this.appHandle!);
      if (st !== 0) {
        hilog.warn(0x0000, 'VK', `drawFrame fail ${st}`);
        return;
      }
      frame++;
      this.msg = `已画 ${frame} 帧`;
    }, 16);
  }
  private stopDraw(): void {
    if (this.timer !== -1) {
      clearInterval(this.timer);
      this.timer = -1;
    }
    if (this.appHandle !== null) {
      vk.destroyVulkanApplication(this.appHandle);
      this.appHandle = null;
      this.msg = '已停止';
    }
  }
}