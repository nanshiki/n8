n8
====

erukiti氏作のVz cloneなテキストエディタ[ne](https://github.com/erukiti/ne)のUTF-8対応版です。  
neは[nice editor](https://ne.di.unimi.it/)と被るため、n8に改名しました。  
LinuxはUbuntu 24.04、AlmaLinux 9、Fedora 41、FreeBSDは13.5、14.2、macOSはMonterey、Sequoiaで動作確認しています。  
Linuxの動作するシングルボードコンピュータRaspberry Pi、Rock 5B、Armadillo 640でも動作確認しています。  
ターミナルはTeraTerm、rlogin、PuTTYで動作確認しています。  

[VZ Editor](https://github.com/vcraftjp/VZEditor)の主要な操作方法・機能を実装しています。ファイラーも装備しておりファイルコピー等が実行できます。  
文字コードJIS/ShiftJIS/EUC/UTF-8を自動判定しUTF-8に変換して編集します。保存時には元の文字コードに戻しますが、文字コードを変更して保存も可能です。  
キーボードマクロはありますが、マクロ言語は搭載していません。画面を分割しての編集はできません。ファイラーの操作体系はFD系です。  

## neからの変更点
- 編集時の文字コードをUTF-8に変更しました。  
- 64bit OSでビルド出来るように修正しました。  
- プロファイル機能を廃止しました。  
- manページ作成を廃止しました。使用方法等は[n8.txt](https://github.com/nanshiki/n8/blob/main/n8.txt)を参照してください。  
- ファイルを指定しないで起動した場合はuntitled.txtを開かずファイル名入力とし、そこでファイル名を入力せずにEnterとした場合はファイラーを開くように変更しました。  
- ファイル名入力時にタブキーでファイル名補完ができるように変更しました。対象ファイルが複数ある場合、シェルのようにリスト表示はせず、タブキー押下で順番に表示します。  
- ファイラーやタブ補完から除外するファイル拡張子の指定を追加しました。  
- ペースト後にカーソル移動するオプションを追加しました。  
- ファイルメニュー等で項目先頭の文字を入力した場合、即座にその項目を実行するように変更しました。  
- [ESC]Q実行時にファイルに変更がある場合、終了してよいか問い合わせするように変更しました。  
- sh_makeによるファイル生成は行わずsh_shells.c, sh_defs.hは生成済のファイルを使用するよう変更しました。  

その他、メッセージの修正等を行っています。  

## ビルド
$ ./configure  
$ make  
$ sudo make install  
LinuxのDebian系はlibncurses-dev、RedHat系はncurses-develを入れてください。  
FreeBSDは特に追加なしでビルドできると思います。  
macOSはXcode、Command line tools for Xcodeをインストールしておいてください。  
ライブラリでncursesやiconvが提供されている環境であれば大体動作すると思われます。  

## ライセンス
GPL v2
