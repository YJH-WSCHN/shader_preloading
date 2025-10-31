import  router  from '@ohos.router';

interface RouterParams {
  imgIndex: number;
}

@Entry
@Component
struct Product {
  private imgIndex: number = (router.getParams() as RouterParams).imgIndex;

  build() {
    Column() {
      Image($r(`app.media.${this.imgIndex}`))
        .width('100%')
        .height('100%')
        .objectFit(ImageFit.Contain)
        .backgroundColor('#000')
        .onClick(() => router.back())
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#000')
  }
}