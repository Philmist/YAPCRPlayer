#pragma once

// volume_state.h — M5.3: 音量クランプ ＋ 最小化連動ミュート状態機械
//
// ヘッダオンリー・純ロジック。UI / mpv / Qt Widget に依存しない。
// テストは tst_shortcuts.cpp から #include するだけで検証できる。

namespace yapcr::app {

// ---------- 音量クランプ / ステップ ----------

// v を 0-100 の範囲にクランプして返す。
inline int clampVolume(int v)
{
    if (v < 0)   return 0;
    if (v > 100) return 100;
    return v;
}

// current + delta を適用して 0-100 にクランプした音量を返す。
inline int applyVolumeStep(int current, int delta)
{
    return clampVolume(current + delta);
}

// ---------- 最小化連動ミュート状態機械 ----------
//
// ミュートには 2 種類の発生源を区別する:
//   userMute_: M キー等でユーザーが明示的にトグルしたミュート
//   autoMute_: minimize_mute 機能による自動ミュート（最小化で ON・復帰で OFF）
//
// mpv の mute プロパティへは effective()（両者の論理和）を反映する。
// これにより:
//   - 手動ミュート中に最小化→復帰しても userMute_ が保持され、ミュートのままになる。
//   - 手動ミュートなしの最小化→復帰では autoMute_ だけが解除され、ミュートも解除される。
class MuteState {
public:
    // ----- 観測 -----
    // ユーザーが明示設定したミュート状態。
    bool userMute()  const { return userMute_; }
    // mpv へ反映すべき実効ミュート（userMute_ || autoMute_）。
    bool effective() const { return userMute_ || autoMute_; }

    // ----- 操作 -----
    // M キー等でユーザーミュートをトグルする。
    void toggleUser() { userMute_ = !userMute_; }
    // 起動時の [state].mute 復元、または外部からの強制セットに使う。
    void setUser(bool m) { userMute_ = m; }

    // 最小化発生時に呼ぶ。
    //   minimizeMuteEnabled = [playback].minimize_mute の現在値。
    //   ユーザーミュート中の最小化には自動ミュートを掛けない（復帰で解除する理由がないため）。
    void onMinimize(bool minimizeMuteEnabled)
    {
        if (minimizeMuteEnabled && !userMute_) {
            autoMute_ = true;
        }
    }

    // 最小化解除（復帰）発生時に呼ぶ。
    //   自動ミュートのみ解除する。ユーザーミュートには触れない。
    void onRestore() { autoMute_ = false; }

private:
    bool userMute_{false};
    bool autoMute_{false};
};

} // namespace yapcr::app
