VideoSudoku
===

Webカメラから入力された数独を解きます。

## 概要
Webカメラの入力映像から数独を検出して、リアルタイムに解を表示するアプリケーションです。

## 依存・動作環境
+ [OpenCV3.1](http://opencv.org)
+ [libsvm](http://www.csie.ntu.edu.tw/~cjlin/libsvm/)
+ Webカメラ

## インストール

``` bash
$ cd ~
$ git clone https://github.com/masaniwasdp/VideoSudoku.git
$ cd VideoSudoku
$ make
```

## 実行

``` bash
$ cd ~/VideoSudoku
$ vs
```

ESCキーを押すとアプリケーションを終了します。

## アンインストール

``` bash
$ cd ~
$ rm -rf VideoSudoku
```

## ライセンス
MITライセンスが適用されます。

[MITライセンス](https://github.com/masaniwasdp/VideoSudoku/blob/master/Licence.txt)

## 作者
+ Sakamoto Kanta
+ Suzuki Shota
+ Yamato Komei