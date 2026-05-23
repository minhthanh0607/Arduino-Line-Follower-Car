# Arduino-Line-Follower-Car
First-year student's project to build a Line Follower Car using Arduino

## 🛠️ Linh kiện sử dụng (Hardware)
* 1x Bo mạch vi điều khiển Arduino.
* 1x Module điều khiển động cơ (Motor Driver).
* 5x Cảm biến hồng ngoại (IR Sensor) để dò line.
* 1x Cảm biến siêu âm đo khoảng cách.
* 2x Động cơ DC giảm tốc kèm bánh xe.
* 3x Nút nhấn (Đỏ, Vàng, Xanh).
* Khung xe, bánh xe mắt trâu, hộp pin và dây cắm (Jumper wires).

## 🔌 Bảng sơ đồ chân (Pin Mapping)

| Linh kiện | Chức năng | Chân Arduino |
| :--- | :--- | :--- |
| **Cảm biến dò line** | Mắt 0, 1, 2, 3, 4 | A1, A2, A3, A4, A5 |
| **Cảm biến siêu âm** | Chân Trig | D3 |
| | Chân Echo | D2 |
| **Động cơ Phải (M1)** | Băm xung (PWM) | D6 |
| | Đảo chiều (DIR) | D7 |
| **Động cơ Trái (M2)** | Băm xung (PWM) | D5 |
| | Đảo chiều (DIR) | D4 |
| **Nút nhấn** | Nút Đỏ (Start / Reset) | D12 |
| | Nút Vàng (Stop) | D13 |
| | Nút Xanh (Đổi chế độ) | A0 |

## 🚀 Tính năng nổi bật của xe
* **Bám line mượt mà:** Sử dụng thuật toán điều khiển PID (Proportional-Integral-Derivative) để tự động điều chỉnh tốc độ 2 bánh xe dựa trên sai số từ 5 mắt cảm biến.
* **Tránh vật cản thông minh:** * Phát hiện vật cản phía trước bằng cảm biến siêu âm. 
  * Kịch bản xử lý tự động: Tạm dừng, lùi nhẹ, đánh lái quẹo phải để né, đi thẳng qua vật cản, vòng trái để tìm lại line và tiếp tục hành trình.
  * Tính năng an toàn: Nếu xe gặp vật cản lần thứ 2, xe sẽ dừng hoạt động hoàn toàn để bảo vệ động cơ.
* **Giao diện điều khiển nút bấm:** Tích hợp 3 nút bấm để chuyển trạng thái hoạt động (bắt đầu chạy, dừng khẩn cấp, hoặc đổi chế độ tốc độ).
* **Giải mê cung (Maze Solving) (Đang phát triển):** Xe có khả năng tự động quay đầu (180 độ) khi đi vào ngõ cụt (không đọc được vạch đen nào).
