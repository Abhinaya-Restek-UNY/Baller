#include "MessageManager.hpp"
#include <cstring>

/* FNV-1a hashing for smaller packet size;
 */
uint32_t hash32(std::string &s) {
  uint32_t hash = 2166136261u; // FNV offset basis

  for (unsigned char c : s) {
    hash ^= c;
    hash *= 16777619u; // FNV prime
  }

  return hash;
}

MessageManager::MessageManager(write_cb_t write_callback,
                               read_cb_t read_callback) {
  this->write = write_callback;
  this->read = read_callback;
}

template <MessagePayload T>
int MessageManager::listen(std::string type,
                           messageListenerCallback<T> callback) {
  uint32_t id = hash32(type);
  if (this->listener_map.find(id) == this->listener_map.end()) {
    this->listener_map.insert(id, {.callback = callback, .size = sizeof(T)});
    return 0;
  }
  return -1;
}

int MessageManager::close(std::string type) {
  uint32_t id = hash32(type);

  if (this->listener_map.find(id) != this->listener_map.end()) {
    this->listener_map.erase(id);
    return 0;
  }
  return -1;
}

template <MessagePayload T>
IO_RETURN MessageManager::send(std::string type, T *data) {
  char *packet = new char[sizeof(T) + sizeof(uint32_t)];
  uint32_t id = hash32(type);
  std::memcpy(packet, &id, sizeof(id));
  std::memcpy(&(packet[sizeof(id)]), data, sizeof(T));
  IO_RETURN ret = this->write(packet, sizeof(T) + sizeof(uint32_t));
  delete[] packet;

  return ret;
};

void MessageManager::poll() {
  IO_RETURN ret = IO_RETURN::WOULD_BLOCK;
  uint32_t id = 0;
  uint32_t more_len = 0;

  while (true) {
    ret = this->read((char *)&id, sizeof(uint32_t), &more_len);
    if (ret == IO_RETURN::MORE) {
      auto listener_it = this->listener_map.find(id);
      if (listener_it == this->listener_map.end()) {
        return;
      }

      if (listener_it->second.size != more_len) {
        return;
      }

      char *data = new char[listener_it->second.size];

      // Shouldnt possible, just sanity check.
      if (this->read(data, listener_it->second.size, &more_len) ==
          IO_RETURN::MORE) {
        this->read(data, -1, &more_len);
        return;
      }

      listener_it->second.callback(data);

      delete[] data;
    } else {
      return;
    }
  }
}
