#pragma once
#include <cstdint>
#include <map>
#include <string>

template <typename T>
concept MessagePayload = std::is_trivially_copyable_v<T>;

template <MessagePayload T> using messageListenerCallback = void (*)(T *);

/*
 * As the name implies
 **/
enum IO_RETURN { SUCCESS = 0, WOULD_BLOCK = 1, ERROR = -1, MORE = 2 };

/*
 * This callback shall be non-blocking and shall not return IO_RETURN::MORE;
 */
using write_cb_t = IO_RETURN (*)(char *data, uint32_t len);

/* thic callback shall be non-blocking.
 *
 * if this callback return IO_RETURN::MORE it will be called again, when buf
 * size is -1, then it shall abolish the current packet. In the case of
 * IO_RETURN::MORE, this callback shall set more_len to be the size of the rest
 * of packet.
 */
using read_cb_t = IO_RETURN (*)(char *data, uint32_t buf_size,
                                uint32_t *more_len);

struct MessageListenerData {
  messageListenerCallback<char> callback;
  uint32_t size;
};

class MessageManager {
public:
  /*
   * Create message manager
   * @param write callback are expected to write in one go. if its gonna block
   * return IO_RETURN::WOULD_BLOCK. it is NOT ALLOWED to return IO_RETURN::MORE
   * @param read callback are expected to be non-blocking. if its gonna block
   * return IO_RETURN::WOULD_BLOCK. otherwise it is allowed to return
   * IO_RETURN::MORE if package are not fully read yet, in this scenario
   * read_callback will be called again untill it finish read. when size_len is
   * -1, then it shall abolish/skip the current packet.
   **/
  MessageManager(write_cb_t write_callback, read_cb_t read_callback);

  /**
   * Register callback to a listener
   *
   * will return 0 when success, -1 if theres already listener.
   *
   * @param message type, be sure that this is the appropriate name for the type
   * accross device.
   * @tparam callback that will be called when theres a message from that type.
   * Data will be deleted after callback finish.
   * @return 0 on successfully register a listener, -1 when listener is already
   * set.
   */
  template <MessagePayload T>
  int listen(std::string type, messageListenerCallback<T> callback);

  /**
   * Close the current listener of the specified type
   *
   * will return 0 when success, -1 if listener is not set
   *
   * @param message type, be sure that this is the appropriate name for the type
   * accross device.
   * @return 0 on successfully register a listener, -1 when listener is already
   * set.
   */
  int close(std::string type);

  /* Send a packet.
   * @param message type, be sure that the type is the appropriate type for the
   * data.
   * @tparam T data to be sent. IT HAS TO BE THE APPROPRIATE DATA.
   * @return read IO_RETURN; this function is non-blocking
   * if this function return IO_RETURN::WOULD_BLOCK call it again till it
   * returns IO_RETURN::SUCCESS to make it blocking. Otherwise theres something
   * wrong with write_callback
   */
  template <MessagePayload T> IO_RETURN send(std::string type, T *data);

  /* This will query new message (i.e calling read_callback) and call the
   * appropriate listener if theres any.
   */
  void poll();

private:
  write_cb_t write;
  read_cb_t read;

  std::map<uint32_t, MessageListenerData> listener_map;
};
