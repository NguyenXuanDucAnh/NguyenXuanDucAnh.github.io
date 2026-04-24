# 📁 Cơ Chế Upload File — EnglishTracker

## Tổng Quan

Dự án sử dụng cơ chế upload file theo kiểu **Base64 over JSON** thay vì `multipart/form-data` thông thường.  
Lý do chuyển sang cách này là thư viện `cpp-httplib v0.43.1` có bug khiến `req.body` luôn rỗng khi nhận `multipart/form-data` từ trình duyệt Chrome (do `Connection: keep-alive`).

---

## 🔄 Luồng Hoạt Động

```
[Trình duyệt]                          [C++ Server]
     |                                       |
     |  1. User chọn file                    |
     |  2. FileReader đọc file → Base64      |
     |  3. Gửi POST /api/upload (JSON)  ---> |
     |                                       |  4. Parse JSON
     |                                       |  5. Base64 decode → binary
     |                                       |  6. Lưu file vào /uploads/
     |  <--- 7. Trả về { ok, filename, size} |
     |  8. Hiển thị link mở file             |
```

---

## 📦 Chi Tiết Từng Bước

### Bước 1–2: Frontend đọc file thành Base64

```javascript
const base64 = await new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result.split(",")[1]);
    reader.onerror = reject;
    reader.readAsDataURL(file);
});
```

- `FileReader.readAsDataURL()` đọc file và trả về chuỗi dạng:  
  `data:image/png;base64,iVBORw0KGgo...`
- `.split(",")[1]` cắt bỏ phần prefix `data:...;base64,` chỉ giữ lại phần data thuần.
- Kết quả là chuỗi Base64 — mã hóa toàn bộ bytes của file thành ký tự ASCII an toàn.

> **Base64 là gì?**  
> Là cách mã hóa dữ liệu nhị phân (binary) thành chuỗi văn bản thuần (text) gồm 64 ký tự: `A-Z`, `a-z`, `0-9`, `+`, `/`.  
> Cứ 3 bytes binary → 4 ký tự Base64. Nên file 100KB sẽ thành ~133KB sau khi encode.

---

### Bước 3: Frontend gửi JSON lên server

```javascript
const response = await fetch("http://localhost:8080/api/upload", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
        filename: file.name,   // "anh.jpg"
        mime_type: file.type,  // "image/jpeg"
        data: base64           // "iVBORw0KGgo..."
    })
});
```

Request body trông như sau:
```json
{
  "filename": "anh.jpg",
  "mime_type": "image/jpeg",
  "data": "iVBORw0KGgoAAAANSUhEUgAA..."
}
```

---

### Bước 4–6: Backend nhận và lưu file

```cpp
svr.Post("/api/upload", [](const httplib::Request &req, httplib::Response &res) {

    // 4. Parse JSON từ req.body
    auto body = json::parse(req.body);
    std::string filename  = body["filename"];
    std::string b64data   = body["data"];

    // 5. Base64 decode → binary
    std::string file_content = base64_decode(b64data);

    // 6. Lưu file binary xuống disk
    std::ofstream ofs("../uploads/" + filename, std::ios::binary);
    ofs.write(file_content.data(), file_content.size());
    ofs.close();
});
```

**Hàm `base64_decode` hoạt động như thế nào?**

```cpp
std::string base64_decode(const std::string& encoded) {
    // Bảng tra cứu: ký tự Base64 → giá trị số (0–63)
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string decoded;
    int val = 0, bits = -8;

    for (unsigned char c : encoded) {
        if (c == '=') break;                    // padding, kết thúc
        auto pos = base64_chars.find(c);        // tra bảng → số 0–63
        if (pos == std::string::npos) continue; // bỏ qua ký tự lạ
        val = (val << 6) + pos;                 // ghép 6 bits vào
        bits += 6;
        if (bits >= 0) {                        // đủ 8 bits → xuất 1 byte
            decoded += (char)((val >> bits) & 0xFF);
            bits -= 8;
        }
    }
    return decoded; // chuỗi binary gốc
}
```

---

### Bước 7: Server trả về kết quả

```json
{
  "ok": true,
  "message": "Upload thành công",
  "filename": "anh.jpg",
  "size": 211420
}
```

---

### Bước 8: Serve file qua HTTP GET

```cpp
svr.Get("/uploads/(.*)", [](const httplib::Request &req, httplib::Response &res) {
    std::string filename = req.matches[1];
    std::ifstream file("../uploads/" + filename, std::ios::binary);
    // ... xác định MIME type theo đuôi file
    res.set_content(content, mime.c_str());
});
```

Truy cập file sau khi upload:
```
http://localhost:8080/uploads/anh.jpg
http://localhost:8080/uploads/document.pdf
http://localhost:8080/uploads/video.mp4
```

---

## ⚠️ Tại Sao Không Dùng multipart/form-data?

`multipart/form-data` là chuẩn HTTP thông thường để upload file. Tuy nhiên:

| Vấn đề | Chi tiết |
|--------|----------|
| **Bug httplib** | `cpp-httplib v0.43.1` không đọc được body của multipart request từ Chrome khi dùng `Connection: keep-alive` |
| **Biểu hiện** | `req.body.size() == 0` dù `Content-Length: 211675` |
| **Nguyên nhân gốc** | httplib chưa xử lý đúng chunked transfer hoặc keep-alive với large body |

**So sánh 2 cách:**

| | multipart/form-data | Base64 over JSON |
|---|---|---|
| Chuẩn HTTP | ✅ Đúng chuẩn | ⚠️ Không phải chuẩn |
| Hoạt động với httplib | ❌ Bug body rỗng | ✅ Hoạt động tốt |
| Overhead | Thấp | ~33% (Base64 phình to hơn) |
| Hỗ trợ file lớn | Tốt hơn | RAM tăng với file lớn |
| Độ phức tạp | Thấp | Trung bình |

---

## 📂 Cấu Trúc Thư Mục

```
Backend/
├── main.cpp          ← Server C++
├── Lib/
│   └── httplib.h     ← cpp-httplib v0.43.1
└── uploads/          ← Thư mục lưu file (tạo thủ công)
    ├── anh.jpg
    └── document.pdf

(Frontend nằm một cấp trên)
index.html
```

---

## 🔒 Bảo Mật Đã Áp Dụng

**Path traversal attack** — Chặn tên file chứa `../` để tránh ghi file ra ngoài thư mục uploads:
```cpp
auto slash = filename.find_last_of("/\\");
if (slash != std::string::npos)
    filename = filename.substr(slash + 1);
```

Ví dụ tấn công bị chặn: filename = `"../../etc/passwd"` → sau khi xử lý còn `"passwd"`.

---

## 🚀 Cải Tiến Trong Tương Lai

Nếu muốn upgrade sau này, có thể chuyển sang các giải pháp sau:

1. **Dùng thư viện khác** như [Crow](https://crowcpp.org/) hoặc [Drogon](https://drogon.org/) — hỗ trợ multipart tốt hơn.
2. **Upgrade httplib** lên bản mới nhất và test lại multipart.
3. **Giới hạn loại file** được upload (whitelist extension).
4. **Đặt tên file theo UUID** để tránh trùng lặp:
   ```cpp
   // filename = uuid + "." + extension
   ```
5. **Giới hạn dung lượng** file upload ở frontend trước khi gửi.
