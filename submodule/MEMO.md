# このディレクトリについて
ビルド対象外場所でsubmoduleをgitから取得するための仮置き場

git submoduleをすると不要ファイルが取れてしまう

src配下に取得するとPlatform.ioがsubmodule単品デバッグ時のコードをビルドしようとしてエラーになる

また、直接submoduleをsrc配下に取得するようにすると設定も消えてしまい面倒

sparse-checkout併用がさっぱり分からないため、ビルド対象外場所に一旦取得して必要ファイルだけコピペするための作業場所を用意する対応とする

# submoduleを更新する手順メモ

1 DLC Starter 側で未プッシュの変更を全てコミット・プッシュする

2 git submodule update をたたく

2 左のgitグラフところにあるリポジトリ選択のボタンで更新したいサブモジュールを選ぶ

3 薄い字のコミットの横を適切なリモートブランチに変える

4 プル

5 DLC_Starter側で submodule のコミットハッシュが更新される差分が出てくるので、コミット先が DLC Starter 側であることを指差し確認してからできる限り単品でコミット・プッシュする

6 submoduleディレクトリに取れたものと MyLibraries　側を WinMerge して反映する

※ごっそり上書きするとDLC Starter 用に変更したパラメータ設定などが全部消える

# submoduleを追加するコマンドメモ

新規追加は gitignore の submodule ディレクトリをコメントアウトしてから実行し、完了後は即コメントアウト解除する

更新は submodule ディレクトリを  gitignore したままで良さそう

## 準備

VS Code の ターミナル を開き、＋アイコンの横の三角ボタンから　GitBash を選択　ルートにChange Directory

## RC_S660_S_DriverForMCU (NFCカードリーダ)

git submodule add https://github.com/hmpow/RC_S660_S_DriverForMCU.git submodule/RC_S660_S_DriverForMCU

## ATP301x_Arduino_SPI (音声合成LSI)

git submodule add https://github.com/hmpow/ATP301x_Arduino_SPI.git submodule/ATP301x_Arduino_SPI

## JpDrvLicNfcCommand (免許証のNFCコマンド)

git submodule add https://github.com/hmpow/JpDrvLicNfcCommand.git submodule/JpDrvLicNfcCommand