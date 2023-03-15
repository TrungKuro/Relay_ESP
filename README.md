# Dự án Relay ESP

### Đôi lời:

> Hiện tại, tới thời điểm 19/01/2023. Dự án có tất cả 3 Version. Với Ver3 có độ hoàn thiện cao nhất.
>
> Mục tiêu của dự án là phát triển một bộ *"điều khiển thiết bị từ xa"* bằng chính **Wifi cục bộ của ESP8266**.
>
> Kết hợp với bộ **Thời gian thực (RTC)**. Thiết bị có thể hoàn toàn thực hiện các chức năng *"hẹn giờ bật tắt theo thời gian thực"* mà không còn phải kết nối với Internet.
>
> Điều này rất tiện ích để lắp đặt ở những khu vực ko có Internet, mà ta vẫn có thể theo dõi, điều khiển thủ công, hay cài đặt cho tự động theo RTC từ xa, ... tất cả chỉ cần qua duy nhất 1 chiếc điện thoại.

### Về Version 1:

> Chủ yếu phần lớn code mình tham khảo từ bài viết này, [ESP8266 NodeMCU Async Web Server – Control Outputs with Arduino IDE (ESPAsyncWebServer library)](https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/).
>
> Để biến ESP8266 thành một *"máy chủ Web (WebServer) không đồng bộ"*, chứa giao diện điều khiển các Relay. Nó sẽ cần sử dụng 2 bộ thư viện `ESPAsyncWebServer` và `ESPAsyncTCP`.
>
> Sau đó mình có thể dùng điện thoại của mình truy cập vào WebServer này với tư cách Client. Để gửi những *"lệnh"* tới ESP, như *"lệnh yêu cầu"* **(HTTP GET)**
>
> Toàn bộ giao diện của WebServer có thể được viết bằng các ngôn ngữ **{ HTML + JavaScript + CSS }**. Tất cả *"văn bản code"* của Web dưới dạng HTML (lồng cả CSS và JavaScript) được lưu trữ trong một biến chuỗi khá đặc biệt, có tên `index_html`, thuộc loại **PROGMEM**.
>
> Biến chuỗi này nó cho phép ta lưu thuần các kí tự văn bản, tuy nhiên nó sẽ khá nhạy cảm với cặp dấu `%%` (là tính năng *"placeholder"*), thứ mà sẽ tạo **Bug** trong Version sau của mình.
>
> May thay phần *"style"* của Web hiện tại ko có bất kỳ kí tự `%` nào để gây **Bug**. Mình sử dụng *"placeholder"* cho dữ liệu **RTC** gồm `%TIME%` và `%DATE%`, cuối cùng cho các **Relay** là `%RELAY%`.
>
> Về cấu trúc lệnh HTTP GET này rất đơn giản, chỉ gồm 2 phần chính:
> 1. Đường dẫn truy cập các nhánh con của WebServer.
> 2. Các cặp giá trị **{ KEY : VALUE }**
>
> Vd: `<ESP_IP>/<ADDRESS>?<KEY_1>=<VALUE_1>&...&<KEY_n>=<VALUE_n>`
> - Bật đèn chân GPIO2,	HTTP GET: /update?output=2&state=1
> - Tắt đèn chân GPIO2,	HTTP GET: /update?output=2&state=0
>
> Cơ bản, phương pháp này đáp ứng được mục tiêu của mình trong việc...
>
> - Tạo 1 WebServer bằng ESP, để cho các thiết bị khác với tư cách Client truy cập, sử dụng bộ Relay này được.
> - Có giao diện cho Client gửi *"lệnh"* đến WebServer, từ đó để ESP có thể điều khiển Relay.
> - Mỗi khi bất kì một Client nào truy cập lần đầu, nó đều có thể lấy nội dung mới nhất hiện tại (tức các trạng thái Relay, hay giá trị RTC) bằng phương pháp *"placeholder"*.
>
> Về bản chất *"placeholder"* giống như một *"điểm đánh dấu"* có thể nằm ở bất cứ đâu trong cấu trúc nội dung Web. Và khi Client yêu cầu WebServer gửi toàn bộ nội dung trang Web cho lần truy cập đầu tiên. ESP có thể dựa theo các giá trị hiện tại mà chọn ra nội dung HTML tương ứng để đắp vào điểm này.
>
> Tuy nhiên nó có điểm yếu, đó là... 
>
> - Giao diện Client này ko thực sự đồng bộ với trạng thái các Relay trên ESP.<br>_ Sự thay đổi trạng thái các Relay ở đây ta thấy là do chính thao tác người dùng lên mỗi Client.<br>_ Chứ nếu một thiết bị Client khác truy cập và điều khiển, Client này sẽ ko biết được bên kia đã làm những gì!<br>_ Cũng như nếu có ai đó nhấn các nút trên bo để điều khiển Relay trực tiếp, các Client cũng chả biết được!
>
> Để khắc phục điều này, mình cần dùng tới phương pháp *"Server-Sent Events"*. Dựa theo bài viết [ESP8266 NodeMCU Web Server using Server-Sent Events (Update Sensor Readings Automatically)](https://randomnerdtutorials.com/esp8266-nodemcu-web-server-sent-events-sse/).
>
> **SSE** cho phép các Client nhận các bản cập nhật tự động từ WebServer thông qua kết nối HTTP. Bất cứ khi nào có *"sự kiện"* mới, ESP sẽ gửi nó đến các Client. Lúc này, giao diện nội dung trang Web trên các Client có thể được cập nhật tự động mà không cần phải thực hiện thêm yêu cầu HTTP nào.
>
> Bên cạnh mình muốn bộ ESP này *"cục bộ"*, tức nó có thể lắp đặt ở bất cứ đâu để sử dụng, mà ko cần quan tâm đến việc có Internet hay ko!
>
> Để làm được vậy, mình dựa theo bài hướng dẫn này, [ESP8266 NodeMCU Access Point (AP) for Web Server](https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/).
>
> Để biến ESP thành một *"Access Point"*, giúp nó tự phát Wifi riêng. Và các Client khác có thể truy cập vào WebServe của nó qua Wifi mỗi khi muốn sử dụng, chỉ bằng 1 lệnh cấu hình đơn giản: `WiFi.softAP(ssid, password);`.
>
> Điểm yếu của giải pháp này là...
>
> - Người dùng phải ở gần quanh vùng phát Wifi của ESP này mới có thể sử dụng được. Và tất nhiên họ ko thể ở một nơi nào đó thật xa, mà lại truy cập điều khiển được ^^!

> **<u>Test thực tế, với 1 Client kết nối:</u>**
>
> Thao tác điều khiển từ Client (Phone) xuống ESP khá mượt. Và cả chiều từ Button đến ESP cũng ko vấn đề. Tuy nhiên, sự đồng bộ trạng thái Relay lên Client, thỉnh thoảng gặp *"stuck"*.
>
> Còn nếu để bộ ESP hoạt động một thời gian lâu, RTC sẽ bị lệch thời gian *(tất nhiên rồi)*. Ở phiên bản hiện tại, chưa có thao tác để Client có thể cập nhập thời gian cho RTC. Mà chỉ có thể cài đặt thời gian cho RTC ở thời điểm nạp code, đương nhiên thời gian này lấy thời điểm biên dịch code, nên tất nhiên sẽ bị lệch thời gian so với thời gian thực khi hoạt động.
>
> Khi Client tạm thời tắt hiển thị, trong lúc này nếu có thao tác nút nhấn, hoặc từ Client khác. Trạng thái các Relay có thể sẽ thay đổi khác ban đầu. Và khi Client này mở hiển thị lại, giao diện trạng thái các Relay có thể ko được cập nhập đồng bộ.

> **<u>Test thực tế, với 2 Client kết nối:</u>**
>
> Lúc này, mình mới thấy sự thiếu đồng bộ giữa các Client. Khi mà Client này điều khiển, tuy có tác động đến Relay trên ESP. Nhưng trạng thái báo Relay trên các Client khác không tương ứng.
>
> Còn nếu mình thao tác trực tiếp nút nhấn trên bộ ESP, thì nó có thể đồng bộ giao diện trạng thái Relay trên cả 2 Client, tuy nhiên thỉnh thoảng vẫn bị *"stuck"*.
>
> **<u>Giải pháp dự định:</u>**
>
> Điều này có lẽ là do mình ko có thêm phần báo *"event"* cho cả thao tác điều khiển Relay trên các Client.

### Về Version 2:

> Chủ yếu chỉnh sửa, cập nhập, bổ sung thêm cho giao diện của WebServer hoàn thiện hơn, như:
>
> - Đổi màu chủ đạo, bố cục cấu trúc giao diện, kích thước chữ, font chữ, hiệu ứng di chuột (chỉ có tác dụng trên PC), nội dung hiển thị, ...
> - Thêm phần nút nhấn *"Update Time"* trên giao diện WebServer giúp Client có thể chủ động cập nhập lại giá trị thời gian cho RTC, với giá trị thời gian lấy từ Client, vốn đã chính xác cao khi luôn được cập nhập mỗi khi nó kết nối Internet.
> - Chỉnh layout dạng chia bảng, với mỗi ô là 1 *"card"*, cho phép vẫn giữa bố cục cân đối, giao diện vẫn trông đẹp mắt, dù có thay đổi kích thước khung trình duyệt Web, cả lẫn trên Phone hay Laptop và PC.
>
> Cuối cùng, để có được một giao diện mới đẹp như vậy. Mình đã tìm ra được nguyên nhân chính gây ra **Bug** trong phần thiết kế **CSS** cho các thẻ *"style"*.
>
> Nguyên nhân đến từ sự hiểu nhầm giữa việc dùng kí tự `%` cho thiết kế giao diện hay cho *"placeholder"*.
>
> Và thủ thuật để **Fix Bug** này, là đem những đoạn dùng CSS sang đoạn dùng HTML, rồi thay kí tự `%` thành `&#37;`. Đây là bộ *"HTML Encoding"*.

> **<u>Test thực tế, với 2 Client kết nối:</u>**
>
> Hiện chỉ vấn đề cài đặt thời gian cho RTC đã được giải quyết. Nhưng các vấn đề còn lại của Version 1 trước vẫn chưa.
>
> Bên cạnh mình để ý, phần lấy dữ liệu thời gian từ RTC của mỗi Client bị chệch nhịp chút (đấy là mới chỉ test trên 2 Client). Điều này có lẽ là do dùng phương thức **HTTP GET**. Khi mà các Client cứ mỗi 1s đều gửi đến WebServer. ESP phải xử lý tiếp nhận yêu cầu của từng Client và gửi ngược giá trị về cho chúng.
>
> **<u>Giải pháp dự định:</u>**
>
> Không sử dụng **HTTP GET** nữa, cho việc cập nhập giá trị thời gian từ RTC đến mỗi Client. Thay vào đó, mình sẽ dùng *"event"* để chính WebServer chủ động cập nhập, thay vì chờ yêu cầu từ mỗi Client.

### Về Version 3:

> Ở bản này, vẫn là chức năng điều khiển các Relay bằng công tắt trên giao diện Web của Client hoặc bằng nút nhấn trực tiếp có trên bộ *"Relay_ESP"*.
>
> Ngoài phần giao diện đã hoàn thiện và các hiệu ứng trong quá trình thao tác điều khiển (chỉ có trên PC).
>
> Điều khác biệt ở đây nữa là sự cải tiến *"tốc độ phản hồi"*, cũng như đảm bảo hoàn toàn *"sự đồng bộ"* trên mọi thiết bị tham gia điều khiển có mặt trong mạng Wifi cục bộ của ESP đó.
>
> **<u>Giải pháp dự định:</u>**
>
> Bên cạnh mình phát hiện, về bản chất biến `index_html` là dạng chuỗi. Vì vậy mình có thể tiết kiệm dung lượng bộ nhớ cho ESP, bằng cách chỉnh sửa lại nội dung cho WebServer. Chủ yếu là bỏ đi các kí tự, nội dung dư thừa. Như các comment hay khoảng trắng.
>
> Test thử xem là khi Client đã truy cập và tạo liên kết với WebServer rồi, thì mỗi khi Client tạm thời tắt trình duyệt. Nếu nó mở lại trình duyệt, nó có gửi yêu cầu load lại cả trang Web ko?<br>Nếu có, ta nên cải thiện hiệu suất cho phần đặt *"placeholder"* của hàm `processor()`. Mấu chốt là làm sao giúp ESP giảm bớt việc phải xử lý nhiều chuỗi, điều này sẽ giúp load Web nhanh hơn.
>
> Với phiên bản sau, nên chỉnh lại *"cấu trúc các file code"*. Nhất là cho phần tính năng `DEBUG` qua Serial. Làm sao cho việc bật/tắt tính năng này dễ dàng, cũng giúp tiết kiệm bộ nhớ kha khá.
>
> Mình cũng phát hiện thêm là vì lý do nào đó, khi dùng thử những phiên bản mới nhất của ESP8266 (hiện tại như `3.0.0` hay `3.1.0`), nó đều làm *"nhiễu nhạy"* khi thao tác với các nút nhấn trên bo. Riêng phiên bản `2.6.0` lại ko bị. Đây cũng là điều mình cần tìm hiểu để cải thiện cho các Ver sau.

### Về Version 4:

> Phiên bản này không có sự nâng cấp tính năng gì mới. Chỉ là tái cấu trúc lại các file, phân chia ra nhiều file code với mỗi vai trò chức năng đặc thù riêng. Điều này có ích cho việc dễ quản lý code, chỉnh sửa nhanh, và nâng cấp nhanh chóng hơn cho sau này.
>
> Tuy vậy, bản này mình cũng có chỉnh sửa chút:
> - Mình đã tìm ra lý do *"nhiễu nhạy"*, thật ra là do phiên bản mới trình biên dịch kiểm tra gắt hơn và nó phát hiện trong hàm `detectButton()` là hàm có *"trả về dữ liệu"*. Nhưng theo logic code, một số trường hợp nếu xảy ra lại ko có giá trị nào để trả về (mặc dù luồng chương trình có lẽ ko bao chạy vào các trường hợp này cả), vì vậy nó báo lỗi.<br>Việc còn lại là mình chỉnh sửa chút code trong hàm này, bằng cách thêm cho nó các giá trị trả về, dù có thể sẽ ko bao giờ dùng tới.
> - Cấu hình thành công ẩn SSID và đổi địa chỉ IP mặc định. Dù cho lúc đầu ko chạy, bởi vì mình mắc 1 lỗi ngớ ngẩn, là phần cấu hình Wifi quăng vào trong Debug và tắt Debug đi.
> - Tham khảo qua việc dùng *"Thanh ghi"* điều khiển trực tiếp cho các Relay. Tuy tốc độ gần như nhanh **x2** (test trên Arduino, đọc ở đơn vị `micro giây`), nhưng có vẻ ko cần thiết phải đổi, vì con người chẳng ai thao tác được nhanh đến vậy.
> - Đã thử nghiệm việc bỏ **HTTP GET** do các *"Client"* mỗi 1s gửi yêu cầu về *"WedServer"* (ESP), thay vào đó sử dụng **SSE** để chính *"WebServer"* tự động gửi cập nhập cho tất cả *"Client"* cứ mỗi 1s.<br>Tuy nhiên có vẻ như nó làm ảnh hưởng đến phản hồi giao diện trên Relay nên mình đã bỏ phương án này.
> - Rút gọn nội dung lượng data truyền giao tiếp giữa Client và WebServer, nhằm tăng tốc độ phản hồi hơn. Bên cạnh mình đã thử đẩy cấu hình ESP lên cao nhất có thể.
> - Đặt lại một số tên biến tránh gây nhầm lẫn và rối. Tối ưu lại cách sử dụng một số biến dữ liệu, nhất là các biến **String** cho cả thành phần Relay, RTC. Tối ưu cả cách sử dụng một số hàm chức năng.
> - Đem phần khởi tạo Serial vào trong Debug luôn. Điều này tránh việc bật Serial mà ko dùng đến, rất lãng phí. Serial sẽ được kích hoạt chung luôn khi bật Debug.
>
> **<u>Giải pháp dự định:</u>**
>
> Có lẽ, nếu muốn tăng tốc độ mượt khi phản hồi hơn, mình cần thiết kế lại chút cấu trúc Website của WebServer. Mục tiêu có thể là cấu hình sao cho nó tương thích trên cả Điện thoại và Máy tính.

### Về Version 5:

> ...

## Danh sách linh kiện & Sơ đồ kết nối

### ESP8266:

> Bo ESP mạch này do nhóm ... phát triển, hiện chưa có thông tin đăng bán trên thị trường.
>
> Sử dụng Vi điều khiển chính là **ESP-WROOM-02U**.
>
> **Lưu ý:** phiên bản thư viện cho **ESP8266 Boards** trên IDE Arduino hiện tại đang sử dụng là `(2.6.0)`.
>
> Thông số đang cấu hình hiện tại trên IDE Arduino:
> - **Board:** Generic ESP8266 Module
> - **Builtin Led:** 13
> - **Upload Speed:** 921600
> - **CPU Frequency:** 160 MHz
> - **Crystal Frequency:** 26 MHz
> - **Flash Size:** 4MB (FS:2MB OTA:~1019KB)
> - **Flash Mode:** QIO (fast)
> - **Flash Frequency:** 80 MHz
> - **Reset Method:** dtr (aka nodemcu)
> - **Debug Port:** Disabled
> - **Debug Level:** None
> - **IwIP Variant:** v2 Lower Memory
> - **VTables:** Flash
> - **Exceptions:** Legacy (new can return nullptr)
> - **Erase Flash:** All Flash Contents
> - **Espressif FW:** nonos-sdk 2.2.1 + 113 (191105)
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