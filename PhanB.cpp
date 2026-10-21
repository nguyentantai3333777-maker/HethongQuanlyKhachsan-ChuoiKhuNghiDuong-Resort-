#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <ctime>

using namespace std;

// ==========================================
// 1. CẤU TRÚC DỮ LIỆU HỖ TRỢ (QUEUE & STACK)
// ==========================================
template <typename T>
class Queue
{
private:
    vector<T> data;

public:
    void enqueue(T item) { data.push_back(item); }
    void dequeue()
    {
        if (!isEmpty())
            data.erase(data.begin());
    }
    T front() const { return data.front(); }
    bool isEmpty() const { return data.empty(); }
    size_t size() const { return data.size(); }
};

template <typename T>
class Stack
{
private:
    vector<T> data;

public:
    void push(T item) { data.push_back(item); }
    void pop()
    {
        if (!isEmpty())
            data.pop_back();
    }
    T top() const { return data.back(); }
    bool isEmpty() const { return data.empty(); }
    size_t size() const { return data.size(); }
};

// ==========================================
// 2. BIỂU DIỄN BÀI VIẾT (POST)
// ==========================================
class Post
{
private:
    int postId;
    int authorId;
    string content;
    string timestamp;

public:
    Post() : postId(0), authorId(0), content(""), timestamp("") {}
    Post(int id, int author, string text, string time)
        : postId(id), authorId(author), content(text), timestamp(time) {}

    int getPostId() const { return postId; }
    int getAuthorId() const { return authorId; }
    string getContent() const { return content; }
    string getTimestamp() const { return timestamp; }

    bool containsKeyword(const string &keyword) const
    {
        return content.find(keyword) != string::npos;
    }

    void display() const
    {
        cout << "[Bai viet #" << postId << "] Tac gia ID: " << authorId
             << " | Thoi gian: " << timestamp << endl;
        cout << "Noi dung: " << content << endl;
        cout << "--------------------------------------------------\n";
    }
};

// ==========================================
// 3. QUẢN LÝ BÀI VIẾT (POSTMANAGER)
// ==========================================
class PostManager
{
private:
    unordered_map<int, Post> postsById;
    unordered_map<int, vector<int>> postsByAuthor;
    int autoPostId;

public:
    PostManager() : autoPostId(1) {}

    int createPost(int authorId, const string &content)
    {
        time_t now = time(0);
        string timeStr = ctime(&now);
        timeStr.pop_back(); // Xóa ký tự newline

        int id = autoPostId++;
        Post newPost(id, authorId, content, timeStr);
        postsById[id] = newPost;
        postsByAuthor[authorId].push_back(id);
        return id;
    }

    bool editPost(int postId, int authorId, const string &newContent)
    {
        if (postsById.find(postId) != postsById.end())
        {
            if (postsById[postId].getAuthorId() == authorId)
            {
                time_t now = time(0);
                string timeStr = ctime(&now);
                timeStr.pop_back();
                postsById[postId] = Post(postId, authorId, newContent, timeStr + " (Da chinh sửa)");
                return true;
            }
        }
        return false;
    }

    bool deletePost(int postId, int authorId)
    {
        if (postsById.find(postId) != postsById.end())
        {
            if (postsById[postId].getAuthorId() == authorId)
            {
                postsById.erase(postId);
                auto &list = postsByAuthor[authorId];
                list.erase(remove(list.begin(), list.end(), postId), list.end());
                return true;
            }
        }
        return false;
    }

    vector<Post> getPostsByUser(int authorId)
    {
        vector<Post> result;
        if (postsByAuthor.find(authorId) != postsByAuthor.end())
        {
            for (int id : postsByAuthor[authorId])
            {
                result.push_back(postsById[id]);
            }
        }
        return result;
    }

    vector<Post> getFeed(const vector<int> &friendIds)
    {
        vector<Post> feed;
        for (int friendId : friendIds)
        {
            vector<Post> userPosts = getPostsByUser(friendId);
            feed.insert(feed.end(), userPosts.begin(), userPosts.end());
        }
        return feed;
    }
};

// ==========================================
// 4. TÌM KIẾM BÀI VIẾT (POSTSEARCHSERVICE)
// ==========================================
class PostSearchService
{
public:
    static vector<Post> searchByKeyword(const vector<Post> &allPosts, const string &keyword)
    {
        vector<Post> result;
        for (const auto &post : allPosts)
        {
            if (post.containsKeyword(keyword))
            {
                result.push_back(post);
            }
        }
        return result;
    }
};

// ==========================================
// 5. QUẢN LÝ ĐỌC GHI FILE (FILEMANAGER)
// ==========================================
class FileManager
{
public:
    static bool saveUsers(const string &filename, const vector<pair<int, string>> &users)
    {
        ofstream outFile(filename);
        if (!outFile.is_open())
            return false;
        for (const auto &u : users)
        {
            outFile << u.first << "," << u.second << "\n";
        }
        outFile.close();
        return true;
    }

    static bool saveFriendships(const string &filename, const vector<pair<int, int>> &edges)
    {
        ofstream outFile(filename);
        if (!outFile.is_open())
            return false;
        for (const auto &e : edges)
        {
            outFile << e.first << "," << e.second << "\n";
        }
        outFile.close();
        return true;
    }
};

// ==========================================
// MAIN DEMO MODULE 3.5 & CONSOLE UI
// ==========================================
int main()
{
    PostManager postManager;

    // Khởi tạo bài viết mẫu
    int p1 = postManager.createPost(101, "Xin chao moi nguoi, day la bai viet dau tien!");
    int p2 = postManager.createPost(102, "Hom nay thoi tiet dep qua, di cafe khong?");
    int p3 = postManager.createPost(101, "C++ va Thuat toan rat thu vi!");

    cout << "=== 3.5. DEMO MODULE UI VA CAC MODULE HO TRO ===\n\n";

    // 1. Hiển thị bài viết của người dùng 101
    cout << "--- BAI VIET CUA USER 101 ---\n";
    vector<Post> userPosts = postManager.getPostsByUser(101);
    for (const auto &post : userPosts)
    {
        post.display();
    }

    // 2. Hiển thị Bảng tin (Feed) của bạn bè (User 101 xem feed gồm bài của User 102)
    cout << "--- BANG TIN (FEED) CUA USER 101 --- \n";
    vector<int> friends = {102};
    vector<Post> feed = postManager.getFeed(friends);
    for (const auto &post : feed)
    {
        post.display();
    }

    // 3. Tìm kiếm bài viết theo từ khóa
    cout << "--- TIM KIEM BAI VIET TU KHOA 'C++' ---\n";
    vector<Post> searchResults = PostSearchService::searchByKeyword(userPosts, "C++");
    for (const auto &post : searchResults)
    {
        post.display();
    }

    // 4. Lưu dữ liệu ra file
    cout << "--- LUU DU LIEU FILE ---\n";
    vector<pair<int, string>> users = {{101, "NguyenVanA"}, {102, "TranThiB"}};
    if (FileManager::saveUsers("users.txt", users))
    {
        cout << "Da luu danh sach nguoi dung vao file 'users.txt' thanh cong!\n";
    }

    return 0;
}