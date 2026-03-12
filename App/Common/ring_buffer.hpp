#pragma once

#include "types.hpp"

// ───── Critical Macro ────────────────────────────────────────────────────────────
#ifdef USE_FREERTOS
    #include "FreeRTOS.h"
    #define RINGBUFF_ENTER_CRITICAL()   taskENTER_CRITICAL()
    #define RINGBUFF_EXIT_CRITICAL()   taskENTER_CRITICAL()
#else
    #define RINGBUFF_ENTER_CRITICAL()  __asm volatile ("cpsid i" ::: "memory")
    #define RINGBUFF_EXIT_CRITICAL()   __asm volatile ("cpsie i" ::: "memory")
#endif

namespace nickel::common {
// ==========================
//        Ring Buffer
// ==========================
/**
 *  @brief Fixed-size, single producer-single consumer, general purpose ring buffer
 *  @note  Use size of power of 2 for masking wrap
 */
template<index_t N, typename T = uint8_t>
class RingBuff {
    static_assert(N > 0, "RingBuff size must be positive");
    static_assert((N & (N-1)) == 0, "RingBuff size must be positive");
    static_assert(std::is_trivially_copyable<T>::value, "RingBuff<T> requires trivially copyable type T");

    private:
        static constexpr index_t MASK = N - 1;

        T buff_[N];
        index_t head_;  // write index
        index_t tail_;  // read index

    public:
        // ───── Constructors ────────────────────────────────────────────────────────────
        RingBuff() : head_(0), tail_(0) {}

        // ───── Write ────────────────────────────────────────────────────────────
        /**
         *  @brief Push a byte into the buffer
         *  @return Status of push operation. BUSY if buffer is full
         *  @note It is the caller's responsibility to check the buffer state before pushing new data.
         */
        Status push(T data) {
            if(isFull()) { return Status::BUSY; }

            buff_[head_ & MASK] = data;
            head_++;
            return Status::OK;
        }

        /**
         *  @brief Push multiple bytes into the buffer
         *  @return Status of push operation. BUSY if buffer does not have enough space for all data
         *  @note It is the caller's responsibility to check the buffer state before pushing new data.
         */
        Status push(const T *data, size_t length) {
            if(writable() < length) { return Status::BUSY; }
            RINGBUFF_ENTER_CRITICAL();
            for(index_t i = 0; i < length; i++) {
                buff_[head_ & MASK] = data[i];
                head_++;
            }
            RINGBUFF_EXIT_CRITICAL();
            return Status::OK;
        }

        /**
         *  @brief Push a string into the buffer
         *  @return Status of push operation. BUSY if buffer does not have enough space for all data
         *  @note It is the caller's responsibility to check the buffer state before pushing new data.
         */
        template<typename U = T>
        typename std::enable_if<std::is_same<U, uint8_t>::value, Status>::type
        push(const char *str, size_t length) {
            return push(reinterpret_cast<const uint8_t *>(str), length);
        }

        // ───── Read ────────────────────────────────────────────────────────────
        /**
         *  @brief Pop a byte from the buffer
         *  @return Status of pop operation. NOT_READY if buffer is empty
         *  @note It is the caller's responsibility to check the buffer state before popping data.
         */
        Status pop(T &data) {
            if(isEmpty()) { return Status::NOT_READY; }

            data = buff_[tail_ & MASK];
            tail_++;
            return Status::OK;
        }

        /**
         *  @brief Peek at the next byte(s) to be popped without removing them from the buffer
         *  @return Pointer to the start of the readable data and its length. nullptr if buffer is empty
         *  @note It is the caller's responsibility to check the buffer state before peeking data.
         */
        const T *peek_data(size_t &length) const {
            size_t idxTail = tail_ & MASK;
            size_t idxHead = head_ & MASK;

            if(isEmpty()) {
                length = 0;
                return nullptr;
            }

            if(idxHead > idxTail) { length = idxHead - idxTail; }
            else { length = N - idxTail; }

            return &buff_[idxTail];
        }

        void consume(size_t length) {
            tail_ += length;
        }
        
        // ───── Check/Set Buffer State ────────────────────────────────────────────────────────────
        /**
         *  @brief Get the buffer size/capacity
         */ 
        static constexpr index_t size() { return N; }

        /**
         *  @brief Check if buffer is empty
         */ 
        bool isEmpty() const { return (head_ == tail_); }
        /**
         *  @brief Check if buffer is full
         */ 
        bool isFull() const { return (writable() == 0); }
        /**
         *  @brief Get how many bytes are readable
         */ 
        index_t readable() const { return (head_ - tail_); }
        /**
         *  @brief Get how many bytes are readable
         */ 
        index_t writable() const { return (N - readable()); }
        /**
         *  @brief Clear the buffer and reset
         */ 
        void clear() { head_ = tail_ = 0; }

};

}   // namespace nickel::common