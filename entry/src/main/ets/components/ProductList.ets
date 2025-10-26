import { ProductCard } from './ProductCard';

@Component
export struct ProductList {
  @Prop total: number = 200;
  @State products: number[] = [];
  private batch = 20;

  aboutToAppear(): void {
    this.loadMore();
  }

  private loadMore(): void {
    const rowsNow = this.products.length;          // 当前已有行数
    const rowsTotal = Math.ceil(this.total / 2);   // 总行数
    if (rowsNow >= rowsTotal) return;              // 到底停

    const rowsToAdd = Math.min(this.batch, rowsTotal - rowsNow);
    for (let i = 0; i < rowsToAdd; i++) {
      this.products.push(rowsNow + i);
    }
  }

  build() {
    List({ space: 12 }) {
      ForEach(this.products, (idx: number) => {
        ListItem() {
          Row({ space: 12 }) {
            ProductCard({ index: idx * 2 });
            ProductCard({ index: idx * 2 + 1 });
          }
          .width('50%')
          .padding({ left: 12, right: 12 });
        };
      }, (idx: number) => idx.toString());
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#f2f2f2')
    .onReachEnd(() => this.loadMore());
  }
}