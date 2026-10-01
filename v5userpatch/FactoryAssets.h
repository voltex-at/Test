#pragma once
#include <Arduino.h>
#include <esp_partition.h>
#include <esp_heap_caps.h>
#include <WiFiClient.h>

// Factory artwork is stored in a dedicated raw flash partition named "assets".
// User artwork stays on microSD and overrides factory artwork at runtime.
namespace factoryassets {
#pragma pack(push,1)
struct Header {
  char magic[8];
  uint32_t version;
  uint32_t count;
};
struct Entry {
  char name[32];
  uint32_t offset;
  uint32_t size;
  uint16_t width;
  uint16_t height;
  uint32_t crc32;
};
#pragma pack(pop)

static const esp_partition_t* partition = nullptr;
static Header header{};
static bool initialized = false;

inline bool begin() {
  partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "assets");
  if (!partition) { Serial.println("Factory assets: partition missing"); return false; }
  if (esp_partition_read(partition, 0, &header, sizeof(header)) != ESP_OK) return false;
  if (memcmp(header.magic, "PEPPA01", 7) != 0 || header.magic[7] != 0 || header.version != 1 || header.count == 0 || header.count > 64) {
    Serial.println("Factory assets: invalid header");
    return false;
  }
  initialized = true;
  Serial.printf("Factory assets: %lu images\n", (unsigned long)header.count);
  return true;
}

inline bool find(const String& name, Entry& out) {
  if (!initialized && !begin()) return false;
  for (uint32_t i=0;i<header.count;++i) {
    Entry e{};
    const size_t pos = sizeof(Header) + i*sizeof(Entry);
    if (esp_partition_read(partition, pos, &e, sizeof(e)) != ESP_OK) return false;
    e.name[sizeof(e.name)-1] = 0;
    if (name == e.name) {
      if (e.size == 0 || e.offset + e.size > partition->size) return false;
      out = e; return true;
    }
  }
  return false;
}

inline bool exists(const String& name) { Entry e{}; return find(name,e); }

inline bool load(const String& name, uint8_t*& data, size_t& size) {
  data=nullptr; size=0; Entry e{}; if(!find(name,e)) return false;
  uint8_t* p=(uint8_t*)heap_caps_malloc(e.size, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!p) p=(uint8_t*)malloc(e.size);
  if(!p) return false;
  if(esp_partition_read(partition,e.offset,p,e.size)!=ESP_OK){free(p);return false;}
  data=p;size=e.size;return true;
}

inline bool streamTo(WiFiClient& client, const String& name) {
  Entry e{}; if(!find(name,e)) return false;
  uint8_t buffer[2048]; uint32_t pos=0;
  while(pos<e.size){
    size_t n=min<size_t>(sizeof(buffer), e.size-pos);
    if(esp_partition_read(partition,e.offset+pos,buffer,n)!=ESP_OK)return false;
    size_t sent=0; while(sent<n){size_t w=client.write(buffer+sent,n-sent);if(!w)return false;sent+=w;}
    pos+=n; delay(0);
  }
  return true;
}

inline size_t sizeOf(const String& name) { Entry e{}; return find(name,e)?e.size:0; }
}
