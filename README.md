# WebLearnEnglish
Website phục vụ cho quá trình học tiếng Anh
# Tham khảo cấu trúc hệ thống theo như tree folder này:

```tree
english-learning-app/
│
├── frontend/                     # Phần Giao diện người dùng (UI)
│   ├── public/                   # Các file tĩnh (index.html, favicon, logo)
│   ├── src/
│   │   ├── assets/               # CSS, CSS frameworks (Tailwind), hình ảnh tĩnh
│   │   ├── components/           # Các thành phần UI dùng chung (Header, Menu, AudioPlayer)
│   │   ├── pages/                # Các trang chính (Dashboard, Roadmap, UploadLesson)
│   │   ├── utils/                # Các hàm tiện ích (format ngày tháng, v.v.)
│   │   └── api/                  # Nơi chứa các hàm gọi API để giao tiếp với Backend
│   ├── package.json              # Khai báo các thư viện UI (nếu dùng React/Vue/Angular)
│   └── README.md
│
├── backend/                      # Phần Xử lý logic (Server)
│   ├── src/
│   │   ├── controllers/          # Xử lý logic nghiệp vụ (VD: userController, lessonController)
│   │   ├── routes/               # Định nghĩa các endpoint API (VD: /api/users, /api/media)
│   │   ├── middlewares/          # Chức năng trung gian (xác thực đăng nhập, xử lý upload file MP3/MP4)
│   │   └── utils/                # Các hàm hỗ trợ đọc/ghi file JSON, parse dữ liệu
│   ├── server.js                 # File gốc khởi chạy server Backend
│   ├── package.json              # Khai báo thư viện Backend (nếu dùng Node.js)
│   └── README.md
│
└── database/                     # Nơi lưu trữ dữ liệu dạng file và media
    ├── users/
    │   └── users.json            # File JSON chứa thông tin, tiến độ học của người dùng
    ├── configs/
    │   ├── system_config.json    # Cấu hình hệ thống (giới hạn dung lượng, thông báo)
    │   └── roadmap_data.json     # Dữ liệu cấu hình cho các lộ trình học
    └── media/                    # Thư mục lưu trữ file bài học
        ├── audio/                # Chứa các file MP3 (Podcast, Talk with AJ, Phát âm)
        ├── video/                # Chứa các file MP4 (Movie, Mini story)
        └── thumbnails/           # Ảnh đại diện cho các video/audio bài học
```