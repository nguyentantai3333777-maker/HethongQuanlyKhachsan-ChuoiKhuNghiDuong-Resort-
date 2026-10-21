# Hotel & Resort Management System — C++

## 1. Giới thiệu dự án

Hotel & Resort Management System là dự án xây dựng một hệ thống quản lý chuỗi khách sạn và khu nghỉ dưỡng cao cấp bằng ngôn ngữ C++, phục vụ mục đích học tập và thực hành các kiến thức về Lập trình hướng đối tượng, Cấu trúc dữ liệu và Giải thuật.

Hệ thống mô phỏng các chức năng vận hành cốt lõi của một chuỗi resort, bao gồm quản lý thông tin khách hàng, đặt phòng, dịch vụ đi kèm, phân bổ và tối ưu hóa hệ thống phòng, cùng với giao diện tương tác qua console.

## 2. Mục tiêu dự án

- Vận dụng kiến thức C++ và lập trình hướng đối tượng (OOP) vào một dự án thực tế.
- Áp dụng các cấu trúc dữ liệu như Hash Table, Graph, Priority Queue và BST (Binary Search Tree).
- Triển khai các thuật toán tìm kiếm, sắp xếp và tối ưu hóa phân bổ phòng.
- Phân chia hệ thống thành các module độc lập để thuận tiện phát triển, kiểm thử và tích hợp.
- Sử dụng Git và GitHub để quản lý mã nguồn và phối hợp làm việc nhóm.

## 3. Phân công chức năng và nhiệm vụ

### 3.1. Module Khách hàng và Bảng băm

**Nhiệm vụ:** Quản lý thông tin khách hàng và xây dựng cơ chế lưu trữ, tra cứu dữ liệu nhanh chóng.

**Các thành phần phụ trách:**
- `Customer`: Đại diện cho thông tin khách hàng.
- `CustomerManager`: Quản lý danh sách và các thao tác với khách hàng.
- `Profile`: Quản lý hồ sơ cá nhân và lịch sử lưu trú.
- `HashTable`: Cấu trúc bảng băm phục vụ lưu trữ và tra cứu.
- `AttributeIndex`: Hỗ trợ lập chỉ mục theo thuộc tính (CCCD/CMND, số điện thoại).

**Thuật toán và kỹ thuật:**
- Hash Table.
- Chaining (xử lý va chạm bằng danh sách liên kết).
- Rehash (xây dựng lại bảng băm khi cần thiết).

**Kết quả cần đạt:** Hỗ trợ thêm, tìm kiếm, cập nhật và quản lý khách hàng; tổ chức dữ liệu hiệu quả để các module khác có thể sử dụng.

### 3.2. Module Đồ thị và Sơ đồ Resort

**Nhiệm vụ:** Mô hình hóa kết nối giữa các khu vực trong resort (phòng nghỉ, nhà hàng, spa, bãi biển) và quản lý lộ trình di chuyển hoặc dịch vụ vận chuyển nội khu.

**Các thành phần phụ trách:**
- `Graph`: Biểu diễn mạng lưới các khu vực trong resort.
- `LocationManager`: Quản lý vị trí và khoảng cách giữa các tiện ích.
- `RouteRequest`: Quản lý yêu cầu điều hướng hoặc xe điện nội khu.
- `ServiceManager::getResortMap()`: Hỗ trợ lấy dữ liệu không gian hiển thị theo thiết kế hệ thống.

**Thuật toán và kỹ thuật:**
- Đồ thị trọng số / Danh sách kề (Adjacency List).

**Kết quả cần đạt:** Quản lý sơ đồ khu nghỉ dưỡng, hỗ trợ định vị và cung cấp dữ liệu không gian cho module tìm kiếm hoặc điều phối dịch vụ.

### 3.3. Module Tìm kiếm và Gợi ý Phòng/Dịch vụ

**Nhiệm vụ:** Tìm kiếm phòng trống theo yêu cầu và đề xuất các gói dịch vụ, tiện ích phù hợp dựa trên sở thích và lịch sử lưu trú của khách hàng.

**Các thành phần phụ trách:**
- `BFSAlgorithm`: Hỗ trợ duyệt tìm đường đi hoặc khu vực lân cận trong resort.
- `RoomSuggester`: Xây dựng chức năng gợi ý phòng và dịch vụ gia tăng.
- `SearchService`: Xử lý các yêu cầu tìm kiếm phòng trống theo tiêu chí.
- `PreferenceAnalyzer`: Phân tích mức độ ưa chuộng dịch vụ của khách hàng.

**Thuật toán và kỹ thuật:**
- BFS (Breadth-First Search).
- Sorting (sắp xếp kết quả theo giá, khoảng cách hoặc độ đánh giá).

**Kết quả cần đạt:** Hỗ trợ tìm kiếm phòng nhanh chóng, duyệt không gian resort và đề xuất dịch vụ theo tiêu chí được thống nhất trong thiết kế.

### 3.4. Module Thống kê và Tối ưu hóa Doanh thu

**Nhiệm vụ:** Phân tích công suất phòng, thống kê doanh thu theo thời gian và đánh giá hiệu suất hoạt động của các chi nhánh trong chuỗi resort.

**Các thành phần phụ trách:**
- `DSU`: Quản lý việc gom nhóm các khu vực hoặc liên kết cụm dịch vụ.
- `DFSAlgorithm`: Duyệt qua các cây phân cấp quản lý hoặc danh mục dịch vụ.
- `RevenueAnalyzer`: Phân tích cấu trúc doanh thu và báo cáo tài chính.
- `OptimizationAlgorithm`: Hỗ trợ tối ưu hóa công suất phòng trống.

**Thuật toán và kỹ thuật:**
- DSU (Disjoint Set Union).
- DFS (Depth-First Search).
- Các thuật toán tính toán và phân loại.

**Kết quả cần đạt:** Cung cấp các chức năng thống kê, phân tích tỷ lệ lấp đầy phòng và các thông tin cấu trúc doanh thu theo yêu cầu của hệ thống.

### 3.5. Module UI và Các module hỗ trợ

**Nhiệm vụ:** Xây dựng giao diện console trực quan và tích hợp toàn bộ các module thành một chương trình quản lý hoàn chỉnh.

**Các thành phần phụ trách:**
- `Booking`: Biểu diễn thông tin đặt phòng.
- `BookingManager`: Quản lý danh sách đặt phòng và nhận/trả phòng (Check-in/Check-out).
- `BookingSearchService`: Hỗ trợ tra cứu lịch sử đặt phòng.
- `FileManager`: Quản lý thao tác đọc và ghi dữ liệu từ tệp (`.txt` hoặc `.csv`).
- `Queue`: Hàng đợi xử lý yêu cầu dịch vụ của khách hàng theo thứ tự.
- `Stack`: Ngăn xếp hỗ trợ hoàn tác (undo) các thao tác đặt phòng/dịch vụ.

**Thuật toán và kỹ thuật:**
- Queue.
- Stack.
- I/O File.
- Console UI.

**Kết quả cần đạt:** Xây dựng menu tương tác, tiếp nhận dữ liệu đầu vào từ người dùng, hiển thị kết quả, lưu trữ dữ liệu an toàn và tích hợp trơn tru các chức năng hệ thống.

## 4. Công nghệ và kiến thức sử dụng

- **Ngôn ngữ lập trình:** C++.
- **Quản lý mã nguồn:** Git và GitHub.
- **Lập trình:** Lập trình hướng đối tượng (OOP).
- **Cấu trúc dữ liệu:**
  - Hash Table (Bảng băm quản lý khách hàng).
  - Graph (Đồ thị quản lý sơ đồ resort).
  - Queue (Hàng đợi xử lý yêu cầu dịch vụ).
  - Stack (Ngăn xếp thao tác hệ thống).
  - DSU (Disjoint Set Union phân tích cụm).
- **Thuật toán:**
  - BFS & DFS (Duyệt không gian và cây phân cấp).
  - Sorting (Sắp xếp danh sách phòng/giá vé).
- **Quản lý dữ liệu:** File I/O (Đọc và ghi tệp lưu trữ).
- **Giao diện người dùng:** Console UI (Giao diện dòng lệnh).

## 5. Quy tắc phát triển và quản lý mã nguồn

Nhóm sử dụng GitHub để lưu trữ mã nguồn, theo dõi lịch sử thay đổi và phối hợp phát triển.

### Quy tắc làm việc

- Mỗi thành viên chịu trách nhiệm cho module được phân công.
- Thống nhất tên hàm, kiểu dữ liệu, tham số và giá trị trả về trước khi triển khai các module.
- Không tự ý sửa đổi giao diện giữa các module khi chưa trao đổi với thành viên liên quan.
- Mỗi thành viên cần kiểm thử chức năng của mình trước khi gửi code tích hợp.
- Các thay đổi quan trọng cần được kiểm tra trước khi đưa vào nhánh chính.

### Quy trình GitHub

1. Đồng bộ code mới nhất từ repository.
2. Tạo hoặc chuyển sang branch phụ trách.
3. Viết code và kiểm thử.
4. Commit các thay đổi với nội dung rõ ràng.
5. Push branch lên GitHub.
6. Tạo Pull Request để trưởng nhóm kiểm tra.
7. Merge vào `main` sau khi được chấp thuận.

### Quy ước đặt tên branch

- `main`: Phiên bản code chính.
- `feature/customer-management`: Module khách hàng và bảng băm.
- `feature/resort-graph`: Module đồ thị và sơ đồ resort.
- `feature/room-suggestion`: Module tìm kiếm và gợi ý phòng.
- `feature/revenue-analysis`: Module thống kê và tối ưu hóa.
- `feature/console-ui`: Module giao diện và tích hợp.

## 6. Cấu trúc thư mục dự kiến

```text
Hotel-ResortManagement/
├── README.md
├── docs/
│   ├── phan-cong-chuc-nang.md
│   └── thiet-ke-he-thong.md
├── include/
│   ├── Customer.h
│   ├── CustomerManager.h
│   ├── Profile.h
│   ├── HashTable.h
│   ├── Graph.h
│   ├── LocationManager.h
│   ├── RoomSuggester.h
│   ├── RevenueAnalyzer.h
│   ├── Booking.h
│   ├── BookingManager.h
│   └── FileManager.h
├── src/
│   ├── main.cpp
│   ├── Customer.cpp
│   ├── CustomerManager.cpp
│   ├── Graph.cpp
│   ├── LocationManager.cpp
│   ├── RoomSuggester.cpp
│   ├── RevenueAnalyzer.cpp
│   ├── Booking.cpp
│   ├── BookingManager.cpp
│   └── FileManager.cpp
└── tests/
    └── README.md
Dưới đây là file `README.md` đã được định dạng chuẩn theo cấu trúc, các đề mục, bảng mã Markdown và phong cách trình bày của dự án Mini Social Network, nhưng toàn bộ nội dung bên trong đã được thay đổi hoàn toàn để chuyển sang đề tài **Hệ thống Quản lý Khách sạn - Chuỗi Khu Nghỉ Dưỡng - Resort**:

```markdown
# Hotel & Resort Management System — C++

## 1. Giới thiệu dự án

Hotel & Resort Management System là dự án xây dựng một hệ thống quản lý chuỗi khách sạn và khu nghỉ dưỡng cao cấp bằng ngôn ngữ C++, phục vụ mục đích học tập và thực hành các kiến thức về Lập trình hướng đối tượng, Cấu trúc dữ liệu và Giải thuật.

Hệ thống mô phỏng các chức năng vận hành cốt lõi của một chuỗi resort, bao gồm quản lý thông tin khách hàng, đặt phòng, dịch vụ đi kèm, phân bổ và tối ưu hóa hệ thống phòng, cùng với giao diện tương tác qua console.

## 2. Mục tiêu dự án

- Vận dụng kiến thức C++ và lập trình hướng đối tượng (OOP) vào một dự án thực tế.
- Áp dụng các cấu trúc dữ liệu như Hash Table, Graph, Priority Queue và BST (Binary Search Tree).
- Triển khai các thuật toán tìm kiếm, sắp xếp và tối ưu hóa phân bổ phòng.
- Phân chia hệ thống thành các module độc lập để thuận tiện phát triển, kiểm thử và tích hợp.
- Sử dụng Git và GitHub để quản lý mã nguồn và phối hợp làm việc nhóm.

## 3. Phân công chức năng và nhiệm vụ

### 3.1. Module Khách hàng và Bảng băm

**Nhiệm vụ:** Quản lý thông tin khách hàng và xây dựng cơ chế lưu trữ, tra cứu dữ liệu nhanh chóng.

**Các thành phần phụ trách:**
- `Customer`: Đại diện cho thông tin khách hàng.
- `CustomerManager`: Quản lý danh sách và các thao tác với khách hàng.
- `Profile`: Quản lý hồ sơ cá nhân và lịch sử lưu trú.
- `HashTable`: Cấu trúc bảng băm phục vụ lưu trữ và tra cứu.
- `AttributeIndex`: Hỗ trợ lập chỉ mục theo thuộc tính (CCCD/CMND, số điện thoại).

**Thuật toán và kỹ thuật:**
- Hash Table.
- Chaining (xử lý va chạm bằng danh sách liên kết).
- Rehash (xây dựng lại bảng băm khi cần thiết).

**Kết quả cần đạt:** Hỗ trợ thêm, tìm kiếm, cập nhật và quản lý khách hàng; tổ chức dữ liệu hiệu quả để các module khác có thể sử dụng.

### 3.2. Module Đồ thị và Sơ đồ Resort

**Nhiệm vụ:** Mô hình hóa kết nối giữa các khu vực trong resort (phòng nghỉ, nhà hàng, spa, bãi biển) và quản lý lộ trình di chuyển hoặc dịch vụ vận chuyển nội khu.

**Các thành phần phụ trách:**
- `Graph`: Biểu diễn mạng lưới các khu vực trong resort.
- `LocationManager`: Quản lý vị trí và khoảng cách giữa các tiện ích.
- `RouteRequest`: Quản lý yêu cầu điều hướng hoặc xe điện nội khu.
- `ServiceManager::getResortMap()`: Hỗ trợ lấy dữ liệu không gian hiển thị theo thiết kế hệ thống.

**Thuật toán và kỹ thuật:**
- Đồ thị trọng số / Danh sách kề (Adjacency List).

**Kết quả cần đạt:** Quản lý sơ đồ khu nghỉ dưỡng, hỗ trợ định vị và cung cấp dữ liệu không gian cho module tìm kiếm hoặc điều phối dịch vụ.

### 3.3. Module Tìm kiếm và Gợi ý Phòng/Dịch vụ

**Nhiệm vụ:** Tìm kiếm phòng trống theo yêu cầu và đề xuất các gói dịch vụ, tiện ích phù hợp dựa trên sở thích và lịch sử lưu trú của khách hàng.

**Các thành phần phụ trách:**
- `BFSAlgorithm`: Hỗ trợ duyệt tìm đường đi hoặc khu vực lân cận trong resort.
- `RoomSuggester`: Xây dựng chức năng gợi ý phòng và dịch vụ gia tăng.
- `SearchService`: Xử lý các yêu cầu tìm kiếm phòng trống theo tiêu chí.
- `PreferenceAnalyzer`: Phân tích mức độ ưa chuộng dịch vụ của khách hàng.

**Thuật toán và kỹ thuật:**
- BFS (Breadth-First Search).
- Sorting (sắp xếp kết quả theo giá, khoảng cách hoặc độ đánh giá).

**Kết quả cần đạt:** Hỗ trợ tìm kiếm phòng nhanh chóng, duyệt không gian resort và đề xuất dịch vụ theo tiêu chí được thống nhất trong thiết kế.

### 3.4. Module Thống kê và Tối ưu hóa Doanh thu

**Nhiệm vụ:** Phân tích công suất phòng, thống kê doanh thu theo thời gian và đánh giá hiệu suất hoạt động của các chi nhánh trong chuỗi resort.

**Các thành phần phụ trách:**
- `DSU`: Quản lý việc gom nhóm các khu vực hoặc liên kết cụm dịch vụ.
- `DFSAlgorithm`: Duyệt qua các cây phân cấp quản lý hoặc danh mục dịch vụ.
- `RevenueAnalyzer`: Phân tích cấu trúc doanh thu và báo cáo tài chính.
- `OptimizationAlgorithm`: Hỗ trợ tối ưu hóa công suất phòng trống.

**Thuật toán và kỹ thuật:**
- DSU (Disjoint Set Union).
- DFS (Depth-First Search).
- Các thuật toán tính toán và phân loại.

**Kết quả cần đạt:** Cung cấp các chức năng thống kê, phân tích tỷ lệ lấp đầy phòng và các thông tin cấu trúc doanh thu theo yêu cầu của hệ thống.

### 3.5. Module UI và Các module hỗ trợ

**Nhiệm vụ:** Xây dựng giao diện console trực quan và tích hợp toàn bộ các module thành một chương trình quản lý hoàn chỉnh.

**Các thành phần phụ trách:**
- `Booking`: Biểu diễn thông tin đặt phòng.
- `BookingManager`: Quản lý danh sách đặt phòng và nhận/trả phòng (Check-in/Check-out).
- `BookingSearchService`: Hỗ trợ tra cứu lịch sử đặt phòng.
- `FileManager`: Quản lý thao tác đọc và ghi dữ liệu từ tệp (`.txt` hoặc `.csv`).
- `Queue`: Hàng đợi xử lý yêu cầu dịch vụ của khách hàng theo thứ tự.
- `Stack`: Ngăn xếp hỗ trợ hoàn tác (undo) các thao tác đặt phòng/dịch vụ.

**Thuật toán và kỹ thuật:**
- Queue.
- Stack.
- I/O File.
- Console UI.

**Kết quả cần đạt:** Xây dựng menu tương tác, tiếp nhận dữ liệu đầu vào từ người dùng, hiển thị kết quả, lưu trữ dữ liệu an toàn và tích hợp trơn tru các chức năng hệ thống.

## 4. Công nghệ và kiến thức sử dụng

- **Ngôn ngữ lập trình:** C++.
- **Quản lý mã nguồn:** Git và GitHub.
- **Lập trình:** Lập trình hướng đối tượng (OOP).
- **Cấu trúc dữ liệu:**
  - Hash Table (Bảng băm quản lý khách hàng).
  - Graph (Đồ thị quản lý sơ đồ resort).
  - Queue (Hàng đợi xử lý yêu cầu dịch vụ).
  - Stack (Ngăn xếp thao tác hệ thống).
  - DSU (Disjoint Set Union phân tích cụm).
- **Thuật toán:**
  - BFS & DFS (Duyệt không gian và cây phân cấp).
  - Sorting (Sắp xếp danh sách phòng/giá vé).
- **Quản lý dữ liệu:** File I/O (Đọc và ghi tệp lưu trữ).
- **Giao diện người dùng:** Console UI (Giao diện dòng lệnh).

## 5. Quy tắc phát triển và quản lý mã nguồn

Nhóm sử dụng GitHub để lưu trữ mã nguồn, theo dõi lịch sử thay đổi và phối hợp phát triển.

### Quy tắc làm việc

- Mỗi thành viên chịu trách nhiệm cho module được phân công.
- Thống nhất tên hàm, kiểu dữ liệu, tham số và giá trị trả về trước khi triển khai các module.
- Không tự ý sửa đổi giao diện giữa các module khi chưa trao đổi với thành viên liên quan.
- Mỗi thành viên cần kiểm thử chức năng của mình trước khi gửi code tích hợp.
- Các thay đổi quan trọng cần được kiểm tra trước khi đưa vào nhánh chính.

### Quy trình GitHub

1. Đồng bộ code mới nhất từ repository.
2. Tạo hoặc chuyển sang branch phụ trách.
3. Viết code và kiểm thử.
4. Commit các thay đổi với nội dung rõ ràng.
5. Push branch lên GitHub.
6. Tạo Pull Request để trưởng nhóm kiểm tra.
7. Merge vào `main` sau khi được chấp thuận.

### Quy ước đặt tên branch

- `main`: Phiên bản code chính.
- `feature/customer-management`: Module khách hàng và bảng băm.
- `feature/resort-graph`: Module đồ thị và sơ đồ resort.
- `feature/room-suggestion`: Module tìm kiếm và gợi ý phòng.
- `feature/revenue-analysis`: Module thống kê và tối ưu hóa.
- `feature/console-ui`: Module giao diện và tích hợp.

## 6. Cấu trúc thư mục dự kiến

```text
Hotel-ResortManagement/
├── README.md
├── docs/
│   ├── phan-cong-chuc-nang.md
│   └── thiet-ke-he-thong.md
├── include/
│   ├── Customer.h
│   ├── CustomerManager.h
│   ├── Profile.h
│   ├── HashTable.h
│   ├── Graph.h
│   ├── LocationManager.h
│   ├── RoomSuggester.h
│   ├── RevenueAnalyzer.h
│   ├── Booking.h
│   ├── BookingManager.h
│   └── FileManager.h
├── src/
│   ├── main.cpp
│   ├── Customer.cpp
│   ├── CustomerManager.cpp
│   ├── Graph.cpp
│   ├── LocationManager.cpp
│   ├── RoomSuggester.cpp
│   ├── RevenueAnalyzer.cpp
│   ├── Booking.cpp
│   ├── BookingManager.cpp
│   └── FileManager.cpp
└── tests/
    └── README.md

## 7. Kiểm thử và tích hợp

Các module cần được kiểm thử riêng trước khi tích hợp. Nhóm cần chú ý:

* Kiểm tra dữ liệu đầu vào không hợp lệ (ngày tháng đặt phòng, số lượng khách).
* Xử lý trường hợp khách hàng, phòng hoặc dịch vụ không tồn tại.
* Kiểm tra dữ liệu trùng lặp (trùng mã đặt phòng, trùng lịch đặt phòng).
* Kiểm tra các trường hợp danh sách phòng rỗng hoặc toàn bộ phòng đã được đặt kín.
* Kiểm tra thao tác đọc, ghi tệp và tình huống tệp dữ liệu không tồn tại.
* Kiểm tra sự tương thích giữa các module khi tích hợp toàn bộ hệ thống.

## 8. Kết quả mong đợi

Hoàn thành một ứng dụng quản lý khách sạn và chuỗi khu nghỉ dưỡng chạy trên console, có khả năng quản lý khách hàng, đặt phòng, dịch vụ, phân tích doanh thu và cung cấp các tính năng tìm kiếm, gợi ý thông minh dựa trên các cấu trúc dữ liệu và thuật toán đã triển khai.

Dự án đồng thời giúp các thành viên rèn luyện kỹ năng lập trình C++, thiết kế kiến trúc phần mềm theo module, kiểm thử hệ thống và làm việc nhóm thông qua GitHub.
