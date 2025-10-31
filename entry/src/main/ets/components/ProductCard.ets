import router from '@ohos.router';

@Component
export struct ProductCard {
  @Prop index: number;

  build() {
    Column() {
      Image($r(`app.media.${(this.index % 200) + 1}`))
        .width('100%')
        .aspectRatio(1)
        .objectFit(ImageFit.Cover)
        .borderRadius(8)

      Text(`商品 ${this.index + 1}`)
        .fontSize(14)
        .width('100%')
        .padding(4)
    }
    .width('100%')
    .backgroundColor(Color.White)
    .borderRadius(12)
    .clip(true)
    .onClick(() => {
      router.push({
        url: 'pages/Product',
        params: { imgIndex: (this.index % 200) + 1 }
      });
    })
  }
}