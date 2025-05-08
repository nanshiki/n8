n8
====

erukiti氏作のVz cloneなテキストエディタ[ne](https://github.com/erukiti/ne)のUTF-8対応版です。  
neは[nice editor](https://ne.di.unimi.it/)と被るため、n8に改名しました。  
LinuxはUbuntu 24.04、AlmaLinux 9、Fedora 41、FreeBSDは13.5、14.2、macOSはMonterey、Sequoiaで動作確認しています。  
Linuxの動作するシングルボードコンピュータRaspberry Pi、Rock 5B、Armadillo 640でも動作確認しています。  
ターミナルはTeraTerm、RLogin、PuTTYで動作確認しています。  

[VZ Editor](https://github.com/vcraftjp/VZEditor)の主要な操作方法・機能を実装しています。ファイラーも装備しておりファイルコピー等が実行できます。  
文字コードJIS/ShiftJIS/EUC/UTF-8を自動判定しUTF-8に変換して編集します。保存時には元の文字コードに戻しますが、文字コードを変更して保存も可能です。  
キーボードマクロはありますが、マクロ言語は搭載していません。  
設定でメニュー枠文字を変更すると DOS/V 版 VZ Editor の外観に近くなります。  

## 画面
![検索](https://www.nanshiki.co.jp/software/n8search.png)
![分割](https://www.nanshiki.co.jp/software/n8teraterm.png)

## neからの変更点
- 編集時の文字コードをUTF-8に変更しました。合成文字(濁点・半濁点のみ)や曖昧文字幅にも対応しています。  
- 64bit OSでビルド出来るように修正しました。  
- プロファイル機能を廃止しました。代替としてファイル履歴とカーソル行の保存機能を追加しています。  
- manページ作成を廃止しました。使用方法等は[n8.txt](https://github.com/nanshiki/n8/blob/main/n8.txt)を参照してください。  
- ファイルを指定しないで起動した場合はuntitled.txtを開かずファイル名入力とし、そこでファイル名を入力せずにEnterとした場合はファイラーを開くように変更しました。  
- ファイル名入力時にタブキーでファイル名補完ができるように変更しました。対象ファイルが複数ある場合、シェルのようにリスト表示はせず、タブキー押下で順番に表示します。  
- ファイルメニュー等で項目先頭の文字を入力した場合、即座にその項目を実行するように変更しました。  
- カーソル行アンダーライン表示、制御文字やタブ文字の色指定、全角スペース表示、ペースト後にカーソル移動等のオプションを追加しました。  
- 分割モード、リードオンリーモード、ファイルの二重化機能を追加しました。   
- ステータスバーにインジケータを表示するように変更しました。システム情報と切り替えるオプションも追加しています。   
- 新規ファイルや検索文字列等はウインドウを表示して入力するように変更しました。   
- ファイラーの操作体系、ファイルクローズやエディタ終了時の動作を VZ Editor と同等になるよう修正しました。   
- メニュー項目や色、ファイラーの色等を DOS/V 版の VZ Editor に近くなるよう修正しました。   
- 英語モードでのメッセージを追加しました。その他いろいろとメッセージ表記を変更しています。  

修正履歴は[ChangeLog.txt](https://github.com/nanshiki/n8/blob/main/ChangeLog.txt)を参照してください。  

## ビルド
$ ./configure  
$ make  
$ sudo make install  
LinuxのDebian系はbuild-essentials、libncurses-dev、RedHat系はgroupinstallで"Development Tools"、installでncurses-develを入れてください。  
FreeBSDは特に追加なしでビルドできると思います。  
macOSはXcode、Command line tools for Xcodeをインストールしておいてください。  
ライブラリでncursesやiconvが提供されている環境であれば大体動作すると思われます。  

## ライセンス
GPL v2
