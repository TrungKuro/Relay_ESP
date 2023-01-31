# Dự án Relay ESP

### Đôi lời:

> Hiện tại, tới thời điểm 19/01/2023. Dự án có tất cả 3 Version. Với Ver3 hiện tại có độ hoàn thiện cao nhất.
>
> Mục tiêu của dự án là phát triển một bộ *"điều khiển thiết bị từ xa"* bằng chính **Wifi cục bộ của ESP8266**.
>
> Kết hợp với bộ **Thời gian thực (RTC)**. Thiết bị có thể hoàn toàn thực hiện các chức năng *"hẹn giờ bật tắt theo thời gian thực"* mà không còn phải kết nối với Internet.
>
> Điều này rất tiện ích để lắp đặt ở những khu vực ko có Internet, mà ta vẫn có thể theo dõi, điều khiển thủ công, hay cài đặt cho tự động theo RTC từ xa, ... tất cả chỉ cần qua duy nhất 1 chiếc điện thoại.

### Về Version 1:

> Chủ yếu phần lớn code mình tham khảo từ bài viết này, [ESP8266 NodeMCU Async Web Server – Control Outputs with Arduino IDE (ESPAsyncWebServer library)](https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/).
>
> Để biến ESP8266 thành một *"máy chủ Web (WebServer) không đồng bộ"*, chứa giao diện điều khiển 4 kênh Relay. Nó sẽ cần sử dụng 2 thư viện `ESPAsyncWebServer` và `ESPAsyncTCP`.
>
> Sau đó mình có thể dùng điện thoại của mình truy cập vào WebServer này với tư cách Client. Để gửi những *"lệnh"* lên ESP, như *"lệnh yêu cầu"* **(HTTP GET)**
>
> Toàn bộ giao diện của WebServer có thể được viết bằng các ngôn ngữ **{ HTML + JavaScript + CSS }**. Tất cả *"văn bản code"* của Web dưới dạng HTML (lồng cả CSS và JavaScript) được lưu trữ trong biến chuỗi khá đặc biệt, có tên `index_html`, thuộc loại **PROGMEM**.
>
> Biến chuỗi này nó cho phép ta lưu thuần các kí tự văn bản, tuy nhiên nó sẽ khá nhạy cảm với cặp dấu `%%` (là tính năng *"placeholder"*), thứ mà sẽ tạo **Bug** trong Version sau của mình.
>
> May thay phần *"style"* của Web hiện tại ko có bất kỳ kí tự `%` nào để gây **Bug**. Mình sử dụng *"placeholder"* cho dữ liệu **RTC** gồm `%TIME%` và `%DATE%`, cuối cùng cho **Relay** là `%RELAY%`.
>
> Về cấu trúc lệnh HTTP GET này rất đơn giản, chỉ gồm 2 phần chính:
> 1. Đường dẫn truy cập
> 2. Cặp giá trị **{ KEY : VALUE }**
>
> Vd: `<ESP_IP>/<ADDRESS>?<KEY_1>=<VALUE_1>&...&<KEY_n>=<VALUE_n>`
> - Bật đèn chân GPIO2,	HTTP GET: /update?output=2&state=1
> - Tắt đèn chân GPIO2,	HTTP GET: /update?output=2&state=0
>
> Cơ bản, phương pháp này đáp ứng được mục tiêu của mình trong việc...
> - Tạo 1 WebServer bằng ESP, để cho các thiết bị khác với tư cách Client truy cập, sử dụng bộ Relay này được.
> - Có giao diện cho Client gửi *"lệnh"* đến WebServer, từ đó để ESP có thể điều khiển Relay.
> - Mỗi khi bất kì một Client nào truy cập lần đầu, nó đều có thể lấy nội dung mới nhất hiện tại (tức các trạng thái Relay) bằng phương pháp *"placeholder"*.
>
> Về bản chất *"placeholder"* giống như một *"điểm đánh dấu"* có thể nằm ở bất cứ đâu trong cấu trúc nội dung Web, và khi Client yêu cầu WebServer gửi toàn bộ nội dung trang Web cho lần truy cập đầu tiên. ESP có thể dựa theo giá trị hiện tại mà chọn ra nội dung HTML tương ứng để đắp vào điểm này.
>
> Tuy nhiên nó có điểm yếu, đó là... 
>
> Giao diện Client này ko thực sự đồng bộ với trạng thái Relay trên ESP. Sự thay đổi ở đây ta thấy là do thao tác người dùng lên chính mỗi Client. Chứ nếu một thiết bị Client khác truy cập và điều khiển, Client này sẽ ko biết được bên kia đã làm gì. Cũng như nếu có ai đó nhấn nút điều khiển Relay trực tiếp, các Client cũng chả biết được.
>
> Để khắc phục điều này, mình cần dùng tới phương pháp *"Server-Sent Events"*. Dựa theo bài viết [ESP8266 NodeMCU Web Server using Server-Sent Events (Update Sensor Readings Automatically)](https://randomnerdtutorials.com/esp8266-nodemcu-web-server-sent-events-sse/).
>
> **SSE** cho phép các Client nhận các bản cập nhật tự động từ WebServer thông qua kết nối HTTP. Bất cứ khi nào có *"sự kiện"* mới, ESP sẽ gửi nó đến Client và trang Web có thể được cập nhật tự động mà không cần thực hiện thêm yêu cầu nào.
>
> Bên cạnh mình muốn bộ ESP này *"cục bộ"*, tức nó có thể lắp đặt ở bất cứ đâu để sử dụng, mà ko cần quan tâm đến việc có Internet hay ko!
>
> Để làm được vậy, mình dựa theo bài hướng dẫn này, [ESP8266 NodeMCU Access Point (AP) for Web Server](https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/).
>
> Để biến ESP thành một *"Access Point"*, giúp nó tự phát Wifi riêng cục bộ. Và các Client khác có thể truy cập vào WebServe của nó qua Wifi mỗi khi muốn sử dụng, chỉ bằng 1 lệnh cấu hình đơn giản: `WiFi.softAP(ssid, password);`.
>
> Điểm yếu của giải pháp này là người dùng phải ở gần quanh vùng phát Wifi của ESP này mới có thể sử dụng được. Và tất nhiên họ ko thể ở một nơi nào đó xa xa, mà lại truy cập điều khiển từ xa được ^^!

> **<u>Test thực tế, với 1 Client:</u>**
>
> Thao tác điều khiển từ Client xuống ESP khá mượt. Chiều ngược lại, từ nút nhấn tới Relay thì ko vấn đề. Nhưng sự đồng bộ trạng thái Relay lên Client, thỉnh thoảng gặp *"stuck"*.
>
> Để một thời gian lâu, RTC sẽ bị lệch thời gian. Ở phiên bản hiện tại, chưa có thao tác để Client yêu cầu cập nhập thời gian cho RTC, mà phải phụ thuộc vào lúc nạp code, dẫn đến thời gian thực tế có sự sai lệch.
>
> Khi Client tắt hiển thị, trong lúc này nếu có thao tác nút nhấn, hoặc từ Client khác. Trạng thái các Relay có thể sẽ thay đổi khác ban đầu. Và khi Client này mở hiển thị lại, giao diện trạng thái các Relay có thể ko được cập nhập đồng bộ.

> **<u>Test thực tế, với 2 Client:</u>**
>
> Lúc này, mình mới thấy sự thiếu đồng bộ giữa các Client. Khi mà Client này điều khiển, tuy có tác động đến Relay trên ESP. Nhưng trạng thái báo Relay trên các Client khác không tương ứng.
>
> Còn nếu mình thao tác trực tiếp nút nhấn trên bộ ESP, thì nó có thể đồng bộ giao diện trạng thái Relay trên cả 2 Client, tuy nhiên thỉnh thoảng vẫn bị *"stuck"*.
>
> Điều này có lẽ là do mình ko có thêm phần báo *"event"* cho cả thao tác điều khiển Relay trên các Client.

### Về Version 2:

> Chủ yếu chỉnh sửa, cập nhập, bổ sung thêm cho giao diện của WebServer hoàn thiện hơn, như:
>
> Đổi màu chủ đạo, kích thước chữ, font chữ, hiệu ứng di chuột, nội dung hiển thị, ...
>
> Thêm phần nút nhấn *"Update Time"* trên giao diện WebServer giúp Client có thể chủ động cập nhập lại giá trị thời gian cho RTC, dựa theo chính giá trị thời gian chính xác hiện tại của các thiết bị Client.
>
> Chỉnh layout dạng lưới *"card"*, cho phép vẫn giữa bố cục giao diện trông đẹp mắt, dù có thay đổi kích thước khung giao diện trình duyệt, cả lẫn trên Phone hay Laptop và PC.
>
> Cuối cùng, để có được một giao diện mới đẹp như vậy. Mình đã tìm ra được nguyên nhân chính gây ra **Bug** trong phần thiết kế **CSS** cho các thẻ *"style"*. Nguyên nhân đến từ sự hiểu nhầm giữa việc dùng kí tự `%` cho thiết kế giao diện hay cho *"placeholder"*.

> **<u>Test thực tế, với 2 Client:</u>**
>
> Các vấn đề của Version 1 trước vẫn còn đó chưa được giải quyết.
>
> Bên cạnh mình để ý, phần lấy dữ liệu thời gian từ RTC của mỗi Client bị chệch nhịp chút (đấy là mới chỉ test trên 2 Client). Điều này có lẽ là do phương thức **HTTP GET**, mà mỗi Client gửi đến. ESP phải xử lý tiếp nhận yêu cầu của từng Client và gửi ngược giá trị về cho chúng.

### Về Version 3:

> Ở bản này, vẫn là chức năng điều khiển 4 Relay bằng Công tắt trên giao diện Web hoặc Nút nhấn có trên bộ *"Relay_ESP"*.
>
> Ngoài phần giao diện đã hoàn thiện và các hiệu ứng trong quá trình thao tác điều khiển.
>
> Điều khác biệt ở đây nữa là sự cải tiến *"tốc độ phản hồi"*, cũng như đảm bảo hoàn toàn *"sự đồng bộ"* trên mọi thiết bị tham gia điều khiển có mặt trong mạng Wifi cục bộ của ESP đó.

### Về Version 4:

> Phiên bản này không có sự nâng cấp tính năng gì mới. Chỉ là tái cấu trúc lại các file, phân chia ra nhiều file code với mỗi vai trò chức năng đặc thù riêng. Điều này có ích cho việc dễ quản lý code, chỉnh sửa nhanh, và nâng cấp nhanh chóng hơn cho sau này.

### Về Version 5:

> ...

## Danh sách linh kiện & Sơ đồ kết nối

### ESP8266:

> Bo ESP mạch này do nhóm ... phát triển, hiện chưa có thông tin đăng bán trên thị trường.
>
> Sử dụng Vi điều khiển chính là **ESP-WROOM-02U**.
>
> **Lưu ý:** phiên bản thư viện cho **ESP8266 Boards** hiện tại đang sử dụng là `(2.6.0)`.
>
> Thông số đang cấu hình hiện tại trên IDE Arduino:
> - **Board:** Generic ESP8266 Module
> - **Builtin Led:** 13
> - **Upload Speed:** 921600
> - **CPU Frequency:** 160 MHz
> - **Crystal Frequency:** 26 MHz
> - **Flash Size:** 1MB (FS:64KB OTA:~470KB)
> - **Flash Mode:** QIO (fast)
> - **Flash Frequency:** 80 MHz
> - **Reset Method:** dtr (aka nodemcu)
> - **Debug Port:** Disabled
> - **Debug Level:** None
> - **IwIP Variant:** v2 Lower Memory
> - **VTables:** Flash
> - **Exceptions:** Legacy (new can return nullptr)
> - **Erase Flash:** All Flash Contents
> - **Espressif FW:** nonos-sdk 2.2.1+111 (191024)
> - **SSL Support:** All SSL ciphers (most compatible)

### Button

> Có tất cả 4 Nút nhấn trên bo mạch *"Relay_ESP"*. Mỗi nút tương ứng điều khiển độc lập 1 Relay.
> - Btn 1 **(0.2 VDC)** = Relay 1
> - Btn 2 **(0.4 VDC)** = Relay 2
> - Btn 3 **(0.6 VDC)** = Relay 3
> - Btn 4 **(0.8 VDC)** = Relay 4
>
> Điểm đặc biệt là tất cả 4 nút nhấn này sử dụng cùng chung 1 chân *"Analog - ADC"* (`TOUT - A0`), thay vì 4 chân *"Digital"* riêng biệt.
>
> Điều này giúp tiết kiệm nhiều chân cho ESP để còn dùng các tính năng khác, bên cạnh có thể mở rộng thêm nút nhấn sau này mà ko cần thêm bất kì chân kết nối nào khác.

### Relay

> Có tất cả 4 kênh Relay trên bo mạch *"Relay_ESP"*. Với thiết kế kích `LOW` sẽ bật Relay, và ngược lại kích `HIGH` sẽ tắt Relay.
> - Relay 1 (Btn 1) <-> `GPIO4 - 4`
> - Relay 2 (Btn 2) <-> `GPIO5 - 5`
> - Relay 3 (Btn 3) <-> `GPIO12 - 12`
> - Relay 4 (Btn 4) <-> `GPIO13 - 13`

### RTC

> Sử dụng **DS1307** với giao thức **I2C**.
> - SCL <-> `GPIO14 - 14`
> - SDA <-> `GPIO2 - 2`
>
> Cần phải cài đặt lại trong code bằng lệnh `Wire.begin(SDA, SCL);` thay vì theo cấu hình mặc định ban đầu.

### LED

> Ngoài 1 **LED nội** (`GPIO13 - 13`) được *"tích hợp sẵn"* trên bo ESP.
> - LED này kích `HIGH` đèn sáng, ngược lại kích `LOW` đèn tắt.
> - Nó cũng sử dụng chung chân GPIO cho 1 kênh Relay trong bo mạch *"Relay_ESP"*.
>
> Ngoài ra trên bo mạch *"Relay_ESP"* còn có kết nối thêm 1 **LED ngoại** (`GPIO16 - 16`).
> - Cũng kích `HIGH` làm đèn sáng, và kích `LOW` làm đèn tắt.
> - LED này có thể sử dụng cho các chức năng Debug. Dựa theo cách chớp tắt LED, ta có thể dễ dàng biết được tình trạng của bo mạch do chính ta quy ước.