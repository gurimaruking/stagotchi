# すたごっち (Stagotchi)

M5Stack で動くたまごっち風バーチャルペット育成ゲーム。
[Stack-chan (ｽﾀｯｸﾁｬﾝ)](https://github.com/meganetaaan/stack-chan) の様々な派生キャラクターを育てよう！

![Platform](https://img.shields.io/badge/platform-M5Stack_Core2-blue)
![Framework](https://img.shields.io/badge/framework-Arduino_(PlatformIO)-green)
![License](https://img.shields.io/badge/license-MIT-yellow)

## 🥚 特徴

- **10種類のキャラクター** — ケアの質としつけ度で進化先が変わる
- **たまごっち準拠のゲームメカニクス** — 空腹・幸福・しつけ・体重・病気・うんち
- **数字当てミニゲーム** — Higher/Lower で遊んで幸福度UP
- **日本語UI** — メニュー・ステータス全て日本語表示
- **セーブ/ロード** — ESP32 NVS に自動保存、電源OFFでも続きから遊べる
- **ダブルバッファ描画** — M5Canvas による滑らかな画面表示

## 🌳 進化ツリー

```
たまご (10秒)
  └→ ベビーチャン (65分)
       └→ チビスタックチャン
            ├─ ケアミス ≤1 → スタックチャンJr.
            │    ├─ しつけ ≧75% → AIスタックチャン (最良)
            │    ├─ しつけ ≧50% → ロスタックチャン
            │    └─ しつけ <50% → タカオ版スタックチャン
            └─ ケアミス ≥2 → ダンボールチャン
                 ├─ しつけ ≧75% → レックスチャン
                 ├─ しつけ ≧50% → プロペラチャン
                 └─ しつけ <50% → DKアトムチャン

🔒 シークレット: SO-ARMチャン
   (AIスタックチャン + 完璧ケア + age ≧ 10h + 20%確率)
```

## 🎮 操作方法

| ボタン | 通常画面 | サブメニュー | ミニゲーム |
|--------|----------|-------------|-----------|
| **A (左)** | カーソル左 | 上移動 | 「低い」 |
| **B (中央)** | 決定 | 選択 | 決定 |
| **C (右)** | カーソル右 | キャンセル | 「高い」 |

## 📋 メニュー (7アイコン)

| アイコン | 機能 | 説明 |
|---------|------|------|
| 食 | 給餌 | ごはん (空腹+1) / おやつ (幸福+1) |
| 灯 | 電気 | 就寝時に電気を消す |
| 遊 | ゲーム | 数字当て Higher/Lower |
| 薬 | 治療 | 病気の時に薬を投与 (1-3回) |
| 掃 | 掃除 | うんちを片付ける |
| 状 | 状態 | ステータス画面表示 |
| 躾 | しつけ | 呼出し時にしかる (+25%) |

## 🛠️ 開発環境

### 必要なもの

- **ハードウェア:** M5Stack Core2
- **開発環境:** [PlatformIO](https://platformio.org/)
- **ライブラリ:**
  - M5Unified (^0.2.2)
  - M5GFX (^0.2.2)

### ビルド & 書き込み

```bash
# PlatformIO CLI
pio run -t upload

# シリアルモニター
pio device monitor
```

### platformio.ini

```ini
[env:m5stack-core2]
platform = espressif32
board = m5stack-core2
framework = arduino
monitor_speed = 115200
upload_speed = 115200
lib_deps =
    m5stack/M5Unified@^0.2.2
    m5stack/M5GFX@^0.2.2
```

## 📁 プロジェクト構成

```
stagotchi/
├── platformio.ini          # PlatformIO ビルド設定
├── include/
│   ├── character.h         # キャラ定義・進化テーブル
│   ├── config.h            # 定数・タイミング設定
│   ├── display.h           # 描画マネージャ
│   ├── game_state.h        # ステートマシン・セーブ/ロード
│   ├── input.h             # ボタン入力抽象化
│   ├── menu.h              # メニュー定義
│   ├── minigame.h          # ミニゲーム
│   ├── pet.h               # ペットデータ構造体
│   ├── sound.h             # サウンドエフェクト
│   └── sprites.h           # 1bit モノクロスプライト (PROGMEM)
└── src/
    ├── main.cpp            # メインループ・状態遷移
    ├── character.cpp        # キャラ定義テーブル・進化ロジック
    ├── display.cpp          # M5Canvas ダブルバッファ描画
    ├── game_state.cpp       # NVS 保存/読込・タイマー再校正
    ├── input.cpp            # M5Unified ボタン処理
    ├── menu.cpp             # メニューカーソル管理
    ├── minigame.cpp         # 数字当てゲーム (5ラウンド)
    ├── pet.cpp              # ステータス管理・減衰・進化・死亡
    └── sound.cpp            # ビープ音パターン・AMP制御
```

## ⚙️ ゲーム仕様

| パラメータ | 値 |
|-----------|-----|
| 空腹 / 幸福 | 0〜4 (ハート表示) |
| しつけ | 0〜100% |
| 体重 | 1〜99 LB |
| 孵化時間 | 10秒 |
| ベビー→チャイルド | 65分 |
| ケアウィンドウ | 15分 (放置でケアミス) |
| うんち発生 | 約3時間ごと (幼少期は短め) |
| 病気 | うんち3個以上で発症リスク |
| 寿命 | 168時間 (7日間) |
| オートセーブ | 60秒ごと |

## 🙏 クレジット

- [Stack-chan](https://github.com/meganetaaan/stack-chan) by [@meganetaaan](https://github.com/meganetaaan) — キャラクターの元ネタ
- [M5Unified](https://github.com/m5stack/M5Unified) / [M5GFX](https://github.com/m5stack/M5GFX) — M5Stack ライブラリ
- たまごっち (バンダイ) — ゲームメカニクスのインスピレーション
