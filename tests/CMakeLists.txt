find_package(Qt6 REQUIRED COMPONENTS Test)

add_subdirectory(tst_net)
add_subdirectory(tst_peercast)
add_subdirectory(tst_watchdog)
add_subdirectory(tst_bbs)
add_subdirectory(tst_display)     # M4.0: 窓サイズ純計算テスト
add_subdirectory(tst_config)      # M5.0: config モジュール（TOML ラウンドトリップ・配置解決）
add_subdirectory(tst_shortcuts)   # M5.1: ActionRegistry / キー正規化（逆引き・Num規約・衝突後勝ち）
