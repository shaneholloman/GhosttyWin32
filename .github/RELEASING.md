# リリース手順

GhosttyWin32 のリリースは GitHub Actions で MSIX をビルド・自己署名 → GitHub Releases にアップロードする。
Scoop / 手動どちらでもインストール可能な配布形態。

## 一回だけやるセットアップ

リポジトリオーナーが最初に一度だけやる作業。証明書の有効期限が切れたら再実行。

### 1. 自己署名証明書を生成

ローカル PowerShell（管理者でなくて良い）で:

```powershell
$cert = New-SelfSignedCertificate `
  -Type CodeSigningCert `
  -Subject "CN=i999rri" `
  -KeyAlgorithm RSA `
  -KeyLength 2048 `
  -HashAlgorithm SHA256 `
  -KeyExportPolicy Exportable `
  -KeyUsage DigitalSignature `
  -CertStoreLocation Cert:\CurrentUser\My `
  -NotAfter (Get-Date).AddYears(5) `
  -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")

$cert.Thumbprint
```

`-Subject` は `Package.appxmanifest` の `Publisher` 属性と**完全に一致**させる必要がある。
今のマニフェストは `CN=i999rri` なのでそのまま使う。変える場合は両方変える。

### 2. PFX (秘密鍵入り) をエクスポート

```powershell
$pwd = Read-Host -AsSecureString -Prompt "PFX password (記録しておく)"
Export-PfxCertificate `
  -Cert "Cert:\CurrentUser\My\$($cert.Thumbprint)" `
  -FilePath ghostty-signing.pfx `
  -Password $pwd
```

### 3. .cer (公開鍵のみ) をエクスポート

```powershell
Export-Certificate `
  -Cert "Cert:\CurrentUser\My\$($cert.Thumbprint)" `
  -FilePath Ghostty.cer
```

### 4. PFX を Base64 化

```powershell
$pfxBytes = [System.IO.File]::ReadAllBytes("ghostty-signing.pfx")
$pfxBase64 = [System.Convert]::ToBase64String($pfxBytes)
$pfxBase64 | Set-Clipboard
Write-Host "PFX base64 をクリップボードにコピーしました"
```

### 5. GitHub Secrets に登録

リポジトリ Settings → Secrets and variables → Actions → New repository secret:

| Secret 名 | 値 |
|---|---|
| `SIGNING_PFX_BASE64` | クリップボードに入った Base64 文字列 |
| `SIGNING_PFX_PASSWORD` | 手順 2 で入力したパスワード |

### 6. Environment "release" を作成

Settings → Environments → New environment → 名前 `release`

推奨保護設定:
- **Deployment branches and tags**: "Selected branches and tags" → `v*` タグのみ許可
- （任意）**Required reviewers**: 自分を指定すれば、リリース直前に手動承認のステップが入る

### 7. `Ghostty.cer` をリポジトリに commit

```powershell
git add Ghostty.cer
git commit -m "Add public signing certificate"
git push
```

公開鍵だけなので機密ではない。配布時にユーザーが取得しやすいようリポジトリに置いておく。

### 8. ローカルの PFX を**安全に削除**

`ghostty-signing.pfx` はもう不要（GitHub Secrets に入っている）。クラウド同期フォルダ等から完全に消す:

```powershell
Remove-Item ghostty-signing.pfx -Force
```

GitHub Secrets を再設定する必要が出たら、手順 1 から証明書を再生成する（同じ Subject を使う限り、ユーザー側の `.cer` 信頼設定もやり直しになる点に注意）。

---

## リリースを切る

```powershell
git tag v0.3.0
git push origin v0.3.0
```

`.github/workflows/release.yml` がトリガーされ:
1. ghostty fork の `feature/swapchain-panel-api` から `ghostty.dll` をビルド
2. `Package.appxmanifest` の `Version` をタグから動的に書き換え (`v0.3.0` → `0.3.0.0`)
3. PFX を Secrets から復元 → MSIX をビルド・署名
4. PFX をすぐに削除（worker の中でも秘密鍵が残らないように）
5. `Ghostty-0.3.0-x64.msix` と `Ghostty.cer` を Releases に upload

Environment 保護で承認が必要にしている場合、ここで手動承認待ちになる。

---

## 証明書の更新 / 失効時

PFX が漏洩した疑い、または有効期限が近づいた場合:

1. **古い証明書を失効** （自己署名なので CRL は無いが、新しい証明書で署名し直して周知）
2. 手順 1〜8 を再実行（同じ Subject `CN=i999rri` を再利用）
3. 新しい `Ghostty.cer` を commit / push
4. 旧証明書で署名された MSIX は **再署名できない**（ユーザー側で旧 cer を信頼から外して新 cer を入れ直す必要あり）

タイミング目安:
- 5年有効で生成 → 4年経過時点で切り替えを検討
- 新リリース時には現在の証明書の有効期限を確認

---

## 運用上の注意

- ❌ ログに secret を出さない（`echo $env:PFX_PASSWORD` 等）
- ❌ サードパーティ Action は固定 SHA で pin（現状の `actions/checkout@v4` 等は GitHub 公式なので OK）
- ✅ Environment 保護を有効化して `v*` タグからしか secret アクセスできないようにする
- ✅ Secrets 漏洩疑いがあれば即座に PFX を再生成して旧 secret を削除

---

## トラブルシューティング

### `MSIX not found under AppPackages/`
msbuild がエラーを起こしている。`Build MSIX package` step のログを確認。

### `Manifest Validation: Publisher does not match`
証明書の Subject (`CN=i999rri`) と manifest の Publisher が不一致。
`Package.appxmanifest:13` の `Publisher` を確認。

### ユーザーから「インストールできない」と報告
Windows のサイドロードが無効になっているか、`Ghostty.cer` を信頼ストアに入れていない可能性。
Settings → Privacy & Security → For developers → "Developer Mode" もしくは
"Sideload apps" を有効化。
