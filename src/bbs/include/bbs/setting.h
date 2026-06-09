#pragma once

#include <QString>

namespace yapcr::bbs {

// setting.txt / setting.cgi 応答の抽出結果
// 移植元: BaseBBS.cpp BBS_TITLE/BBS_NONAME_NAME/BBS_THREAD_STOP 展開 (@125-135, 802-818)
struct SettingInfo {
    QString title;       // BBS_TITLE（板タイトル）
    QString noname;      // BBS_NONAME_NAME（デフォルト名無し）
    int     stop{1000};  // BBS_THREAD_STOP（終了レス数、既定 1000）
};

// 復号済みテキスト（setting.txt / setting.cgi 応答）をパースして SettingInfo を返す。
// jpnkn / したらば とも書式は同一（name=value 行）なので BoardType 不要。
// 移植元: BaseBBS::parser(name,value)@592
SettingInfo parseSetting(const QString& text);

}  // namespace yapcr::bbs
