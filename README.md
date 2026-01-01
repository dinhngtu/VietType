# VietType—Bộ gõ tiếng Việt nguồn mở

## VietType là gì?
VietType là bộ gõ tiếng Việt nguồn mở dành cho Windows 10.

![viettype](/viettype.png)

## Tại sao sử dụng VietType?
- VietType là phần mềm nguồn mở. Bất cứ ai có thể sử dụng, sửa đổi và lưu hành VietType theo Giấy phép Công cộng GNU phiên bản 3 (GNU GPL v3).
- Tôi phát triển VietType theo tiêu chí thiết kế: tăng cường bảo mật, tôn trọng quyền riêng tư, không quảng cáo, không theo dõi người dùng, minh bạch về mã nguồn để cộng đồng có thể kiểm tra tính an toàn.
  - Lưu ý: Không phần mềm nào có thể bảo đảm bảo mật 100%; bạn phải kiểm tra bản tải về của VietType theo mục "Lưu ý quan trọng" ở dưới.
- Tôi cung cấp VietType hoàn toàn miễn phí, không vì lợi lộc.
- VietType sử dụng phương thức nhập liệu có sẵn của Windows nên tương thích với nhiều ứng dụng như Chrome, Skype, HTKK, v.v.

## Tôi tải VietType ở đâu?
Tải VietType từ nguồn chính thức duy nhất: https://github.com/dinhngtu/VietType/releases

### **Lưu ý quan trọng:**
Bản quyền © 2018 Đinh Ngọc Tú. **VietType được cung cấp dưới Giấy phép Công cộng GNU phiên bản 3 (GNU GPL v3).** Nếu bạn lưu hành VietType hay bản sửa đổi của VietType, bạn phải cung cấp kèm theo Giấy phép này cũng như mã nguồn và mọi thay đổi mã nguồn lên VietType. Mọi thay đổi lên VietType đều phải sử dụng cùng giấy phép với VietType (GNU GPL v3).

**VietType được cung cấp nguyên trạng, không bao gồm bất cứ bảo hành hay bảo đảm nào.** Bạn tự chấp nhận mọi rủi ro và trách nhiệm khi sử dụng VietType. Bạn cần kiểm chứng độ an toàn của VietType bằng những cách sau:
  - Chỉ tải VietType từ trang web chính thức: https://github.com/dinhngtu/VietType
  - Kiểm tra mã hash của các file tải về
  - Kiểm tra mã nguồn của phiên bản VietType tương ứng
  - Kiểm tra "chứng nhận xuất xứ" (provenance attestation) của GitHub Actions trong mỗi build của VietType (v1.11.1 trở lên)

## Sử dụng VietType như thế nào?
Chạy file MSI để tiến hành cài đặt. Sau khi cài đặt xong, bạn mở chương trình **VietType Settings** trong thư mục Start. Nhấn *Enable VietType/Bật VietType* để kích hoạt VietType.

VietType sử dụng cách gõ Telex tự do. VietType thể hiện trạng thái trên thanh ngôn ngữ của Windows bằng biểu tượng "hình tròn có dấu chấm" (bật tiếng Việt) hoặc "hình tròn rỗng" (tắt tiếng Việt). Các phím tắt của VietType như sau:
- Alt-huyền (nút bên trên nút Tab) để bật và tắt VietType;
- Ctrl (trong lúc gõ từ): Kết thúc từ ngay lập tức;
- Esc (trong lúc gõ từ): Huỷ bỏ gõ từ, chuyển về phím được gõ ban đầu.

Chương trình VietType Settings cho phép cài đặt lựa chọn của VietType. VietType có những lựa chọn sau:

**Lựa chọn kiểu gõ:**
- *Typing style/Kiểu gõ:* lựa chọn kiểu gõ Telex hoặc VNI;
- *Use "oà", "uý" instead of "òa", "úy"/Gõ "oà", "uý" thay vì "òa", "úy:"* lựa chọn cách bỏ dấu cho các vần "oà", "uý", v.v.;
- *Accept 'd' anywhere/Gõ 'đ' tự do:* cho phép gõ "d" để chuyển "d" -> "đ" tại bất cứ nơi nào trong từ;
- *Backspaced invalid word stays invalid/Giữ trạng thái sai khi xóa từ sai:* quy định có giữ tiếng Việt khi xóa từ sai hay không;
- *Multilingual typing optimization level/Mức tối ưu gõ nhiều ngôn ngữ:* thay đổi phương pháp gõ để phù hợp với người thường xuyên gõ nhiều ngôn ngữ (đặc biệt là tiếng Anh);
- *Enable autocorrect/Bật tự sửa từ*: Bật một số phương pháp sửa từ tự động khi gõ sai (ví dụ: "miếgn" -> "miếng").

**Lựa chọn hệ thống:**
- *Vietnamese mode by default/Mặc định bật tiếng Việt:* bật/tắt tiếng Việt khi khởi động máy tính;
- *Word reconversion mode/Chế độ chuyển ngược từ:* cho phép sửa từ sau khi đã gõ xong từ đó;
- *Toggle key/Phím chuyển:* chọn phím chuyển Anh/Việt;
- *Current word highlight mode/Gạch chân từ đang gõ:* chọn kiểu gạch chân dưới từ đang gõ.

## Câu hỏi thường gặp
- Tại sao VietType ít tính năng vậy? Tại sao VietType không có các tính năng như gõ tắt, chuyển mã, v.v.? \
Tôi thiết kế VietType để đáp ứng nhu cầu gõ tiếng Việt tối thiểu.

- Tại sao có UniKey, EVKey, OpenKey... rồi lại sinh ra VietType nữa?
  - VietType dựa trên TSF là nền tảng bộ gõ chính thống của Windows (không giống các bộ gõ trên dùng phương pháp backspace), rất giàu tính năng hỗ trợ cho cả bộ gõ và các ứng dụng khác (trình duyệt, trình soạn thảo...)
  - Nhờ vậy VietType có thể tự tắt khi gặp textbox gõ password, số điện thoại, v.v.
  - VietType không gặp phải bug Chrome/HTKK mà các bộ gõ khác từng gặp.
  - VietType có thể đọc trực tiếp text buffer của ứng dụng, vì vậy tính năng sửa từ hoạt động ổn định hơn rất nhiều so với các bộ gõ khác.
  - VietType không xung đột với các bộ gõ cho ngôn ngữ khác (Trung/Nhật/Hàn). Các bộ gõ tiếng Việt khác như OpenKey có thể đọc trực tiếp trạng thái của VietType bằng TSF API.
  - VietType không cần cài đặt global hook, không cần chạy với quyền admin (chỉ cần lúc cài). Vì vậy tính bảo mật và cách ly dữ liệu giữa các app sẽ cao hơn.
  - Mỗi nền tảng bộ gõ đều có ưu và nhược điểm riêng. VietType chỉ đơn giản cho người dùng thêm một lựa chọn.
