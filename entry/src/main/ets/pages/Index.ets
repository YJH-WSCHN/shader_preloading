import { ProductList } from '../components/ProductList';

@Entry
@Component
struct Index {
  build() {
    Column() {
      Row() {
        Text('华为应用商城')
          .fontSize(24)
          .fontColor('#fff');
      }
      .width('100%')
      .height(56)
      .backgroundColor('#ff3b8eed')
      .justifyContent(FlexAlign.Center);

      ProductList({ total: 200 })
        .layoutWeight(1);
    }
    .width('100%')
    .height('100%');
  }
}