VideoSudoku
===

Webカメラから入力された数独を解きます。

## 概要
Webカメラの入力映像から数独を検出して、リアルタイムに解を表示するアプリケーションです。

## 依存・動作環境
+ [OpenCV3.1](http://opencv.org)
+ [libsvm3.21](http://www.csie.ntu.edu.tw/~cjlin/libsvm/)

## ビルド

``` bash
$ cd ~
$ git clone https://github.com/masaniwasdp/VideoSudoku.git
$ cd VideoSudoku
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=release
$ make
```

## 実行

``` bash
$ cd ~/VideoSudoku/build
$ ./videosudoku
```

SPACEキーを押すと画面表示を固定します。
また、ESCAPEキーを押すとアプリケーションを終了します。

## ライセンス
[MITライセンス](https://github.com/masaniwasdp/VideoSudoku/blob/master/Licence.txt)が適用されます。

## 作者
+ Sakamoto Kanta
+ Suzuki Shota
+ Yamato Komei
