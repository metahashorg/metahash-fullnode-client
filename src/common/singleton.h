#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <class T>
class singleton
{
public:
    static T* get() {
        static T* inst = new T;
        return inst;
    }

    static void free() {
        if (get()) {
            delete get();
        }
    }

    T* operator->() { return get(); }
    const T* operator->() const { return get(); }

protected:
    singleton() {}
    ~singleton() {}

private:
    singleton(const singleton&) = delete;
    singleton(const singleton&&) = delete;

    singleton& operator=(const singleton&) = delete;
    singleton& operator=(singleton&) = delete;
    singleton& operator=(const singleton&&) = delete;
    singleton& operator=(singleton&&) = delete;
};

#endif // __SINGLETON_H__
