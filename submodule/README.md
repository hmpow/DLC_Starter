# このディレクトリについて
git submoduleをすると不要ファイルが取れてしまう

src配下に取得するとPlatform.ioがビルドしようとしてエラーになる

sparse-checkout併用がさっぱり分からないため、ビルド対象外場所に一旦取得して必要ファイルだけコピペするための作業場所を用意する

# gitから取ってくる方法



# gitコマンドのメモ

準備：ターミナル　＋の横の三角　GitBash を選択　ルートにChange Directory

gitignore の submodule をコメントアウトしてから実行し、完了後は即コメントアウト解除する

## RC_S660_S_DriverForMCU (NFCカードリーダ)

git submodule add https://github.com/hmpow/RC_S660_S_DriverForMCU.git submodule/RC_S660_S_DriverForMCU

## ATP301x_Arduino_SPI (音声合成LSI)

git submodule add https://github.com/hmpow/ATP301x_Arduino_SPI.git submodule/ATP301x_Arduino_SPI

## JpDrvLicNfcCommand (免許証のNFCコマンド)

git submodule add https://github.com/hmpow/JpDrvLicNfcCommand.git submodule/JpDrvLicNfcCommand