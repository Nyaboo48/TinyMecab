# Tiny Mecab

勉強のためにMeCabの最低限の機能を抜き出した最低なプログラムです。

- C++20で記述
- 解析機能のみ(辞書作成・学習機能なし)
- 文字コードは辞書も入力テキストもUTF-8に限定
- 辞書はシステム辞書(sys.dic)のみ対応、ユーザー辞書は非対応
- パーシャル解析は削除

It is a minimal program that extracts the minimum functionality of MeCab for study purposes.

- Written in C++20
- Only parsing function (no dictionary creation/learning function)
- Character encoding is limited to UTF-8 for both dictionary and input text
- Only system dictionary (sys.dic) is supported, user dictionary is not supported
- Partial analysis is removed.

## 動作環境

WSL2上のDebianとclang++ 18.0.0でコンパイルし、mecab 0.996.6と結果を比較してだいたい合っているような感じになっています。

- MeCab 0.996.6
- Unidic-csj-202302
- Debian in WSL2
- clang++ 18.0.0

## C++20

C++20の勉強のために書いたため、C言語とのインターフェース部分は完全に削除しています。
文字列の管理をstd::stringでできるのでとても楽です。

## UTF-8

辞書も入力テキストも、なんのチェックもせずにUTF-8として扱っています。

## 辞書

システム辞書(sys.dic)のみ対応しています。
自分ではユーザー辞書は使わずにシステム辞書を再構築して使っているので。

## パーシャル解析を削除するとはなにごとだ

「外国人参政権」のような切れ目のわかりにくい語は「外国(がいこく)人参(にんじん)政権(せいけん)」になりがちです。
パーシャル解析では語の切れ目を指定することができます。
しかし、パーシャル解析を使わずとも「外国^人参^政権」のようにテキトーな区切り文字を挟むことで語の境界は指定できるので、ものぐさな私はそれでやり過ごしています。品詞などを与えることはできませんが。
ASCII CODE 0x1E(RS: レコード区切り)は、vimならCtrl-V, Ctrl-^で入力できます。
