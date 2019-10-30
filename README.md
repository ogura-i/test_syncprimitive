# test_syncprimitive
## 概要
CPUの提供する同期プリミティブ命令のxaddを用いたスピンロック関数とその動作確認プログラム

## プログラム一覧
+ inter_kernel/
  + my_spinlock_k.h
    + カーネル間で排他制御を行うスピンロック関数やxadd命令の定義ファイル
  + increment_inter_kernel_spinlock0.c, increment_inter_kernel_spinlock1.c
    + カーネル間でMintの持つ共有メモリ上の値をインクリメントする処理に対して，スピンロック関数を用いて排他制御できているかの確認モジュール
    + increment_inter_kernel_spinlock0.c: 先行モジュール
    + increment_inter_kernel_spinlock1.c: 後続モジュール
  + increment_inter_kernel_xadd0.c, increment_inter_kernel_xadd1.c
    + カーネル間でMintの持つ共有メモリ上の値をxadd命令でインクリメントし，xadd命令がアトミックに行われているかの確認モジュール
    + increment_inter_kernel_xadd0.c: 先行モジュール
    + increment_inter_kernel_xadd1.c: 後続モジュール
+ inter_process/
  + my_spinlock_u.h
    + プロセス間で排他制御を行うスピンロック関数やxadd命令の定義ファイル
  + increment_in_thread_spinlock.c
    + 4つのスレッド間で1つのグローバル変数をインクリメントする処理に対して，スピンロック関数を用いて排他制御可能か否かの確認プログラム
  + increment_in_thread_spinlock.c
    + 4つのスレッド間で1つのグローバル変数をxadd命令でインクリメントし，xadd命令がアトミックに行われているかの確認プログラム

## 現状
+ プロセス間では，スピンロック関数を使って排他制御可能
+ カーネル間でMintの持つ共有メモリ上の値をxadd命令でインクリメントした場合，xadd命令がアトミックに行えていない．
