Tự sửa từ và tối ưu gõ nhiều ngôn ngữ
=====================================

Xem [`TelexEngine.cpp`](/Telex/TelexEngine.cpp) để biết logic cụ thể của 2 tính năng này.

Tự sửa từ
---------

- (Telex) "wu" = "ưu"
- (Telex) "wo" = "ơ" (khi có phụ âm đầu)
- (Telex) "wuo" = "ươ"
- "ie" + dấu sắc/nặng = "iê" (khi có phụ âm đầu+cuối)
- "ah"/"êh" + dấu sắc/nặng = "ach"/"êch"
- "ah"/"êh" + không dấu = "anh"/"ênh" (Optimize=0)
- "gn" = "ng"
- "g" = "ng" (Optimize=0)

Tối ưu nhiều ngôn ngữ
---------------------

Mục đích chính của thiết lập "tối ưu gõ nhiều ngôn ngữ" là đơn giản hóa việc gõ tiếng Việt và tiếng Anh cùng lúc.

- Mức 0: tắt hoàn toàn tối ưu tiếng Anh
- Mức 1:
  - Tắt tiếng Việt khi gặp một số từ tiếng Anh có 2 dấu (xem `wlist_en`, vd. "bo**x**e**r**")
  - Tắt tự sửa từ đối với 1 số từ tiếng Anh (xem `wlist_en_ac`, vd. "hash" != "hách"
  - Tắt 1 số quy tắc tự sửa từ (xem trên)
- Mức 2: mức 1 và đồng thời
  - Cấm thêm 1 số từ nữa (xem `wlist_en_2`)
- Mức 3: mức 2 và đồng thời
  - Không gõ nguyên âm sau khi gõ dấu (vd. "dense" != "dến")
