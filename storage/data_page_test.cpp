#include <gtest/gtest.h>
#include <memory>
#include <iostream>

#include "common/comparator.h"
#include "page_builder.h"
#include "storage/data_page_builder.h"
#include "storage/data_page_reader.h"

namespace litelsm {

class IntegerComparator : public Comparator {
 public:
  IntegerComparator() = default;

  virtual ~IntegerComparator() = default;

  int compare(const Slice& a, const Slice& b) const override {
    int aVal = decode_fixed32_le((const uint8_t*) a.data());
    int bVal = decode_fixed32_le((const uint8_t*) b.data());
    if (aVal == bVal) {
      return 0;
    }
    if (aVal < bVal) {
        return -1;
    } else {
        return 1;
    }
  }

  const char* Name() const override {
    return "litelsm.IntegerComparator";
  }
};

TEST(DataPageTest, basic) {
    std::vector<std::pair<int, int>> data;
    for (int i = 0; i < 1024; i++) {
        data.push_back(std::make_pair(i, i + 1024));
    }
    DataPageBuilder builder;
    size_t pageSize = builder.pageSize();
    ASSERT_EQ(4 * 1024, pageSize);
    int added = 0;
    for (auto& kv : data) {
        Slice key((const char*) &kv.first, sizeof(int));
        Slice value((const char*) &kv.second, sizeof(int));
        if (builder.estimateSize() + DataPageBuilder::estimateEntrySize(key, value) > pageSize) {
            break;
        }
        builder.add(key, value);
        added++;
    }
    ASSERT_EQ(added, builder.getRecordNum());
    const Slice& page = builder.finish();
    ASSERT_LE(page.getSize(), pageSize);

    DataPageReader reader(page);
    ASSERT_EQ(true, reader.checkCRC32C());
    ASSERT_EQ(PageType::kDataPage, reader.getPageType());
    Iterator* iter = reader.newIterator(new IntegerComparator());
    iter->seekToFirst();
    ASSERT_TRUE(iter->valid());
    int read = 0;
    while (iter->valid()) {
        const Slice& key = iter->key();
        ASSERT_EQ(sizeof(int), key.getSize());
        const Slice& value = iter->value();
        ASSERT_EQ(sizeof(int), value.getSize());
        int parsedKey = decode_fixed32_le((const uint8_t*) key.data());
        ASSERT_EQ(data[read].first, parsedKey);
        int parsedValue = decode_fixed32_le((const uint8_t*) value.data());
        ASSERT_EQ(data[read].second, parsedValue);
        iter->next();
        read++;
    }
    ASSERT_EQ(added, read);

    iter->seekToLast();
    ASSERT_TRUE(iter->valid());
    read = 0;    
    while (iter->valid()) {
        const Slice& key = iter->key();
        ASSERT_EQ(sizeof(int), key.getSize());
        const Slice& value = iter->value();
        ASSERT_EQ(sizeof(int), value.getSize());
        int parsedKey = decode_fixed32_le((const uint8_t*) key.data());
        ASSERT_EQ(data[added - 1 - read].first, parsedKey);
        int parsedValue = decode_fixed32_le((const uint8_t*) value.data());
        ASSERT_EQ(data[added - 1 - read].second, parsedValue);
        iter->prev();
        read++;
    }
    ASSERT_EQ(added, read);

    int target = 200;
    Slice targetSlice((const char*) &target, sizeof(int));
    iter->seek(targetSlice);
    ASSERT_TRUE(iter->valid());
    ASSERT_EQ(target, decode_fixed32_le((const uint8_t*) iter->key().data()));
    while (iter->valid()) {
        const Slice& key = iter->key();
        ASSERT_EQ(sizeof(int), key.getSize());
        const Slice& value = iter->value();
        ASSERT_EQ(sizeof(int), value.getSize());
        int parsedKey = decode_fixed32_le((const uint8_t*) key.data());
        ASSERT_EQ(data[parsedKey].first, parsedKey);
        int parsedValue = decode_fixed32_le((const uint8_t*) value.data());
        ASSERT_EQ(data[parsedKey].second, parsedValue);
        iter->next();
    }
    iter->seek(targetSlice);
    ASSERT_TRUE(iter->valid());
    ASSERT_EQ(target, decode_fixed32_le((const uint8_t*) iter->key().data()));
    while (iter->valid()) {
        const Slice& key = iter->key();
        ASSERT_EQ(sizeof(int), key.getSize());
        const Slice& value = iter->value();
        ASSERT_EQ(sizeof(int), value.getSize());
        int parsedKey = decode_fixed32_le((const uint8_t*) key.data());
        ASSERT_EQ(data[parsedKey].first, parsedKey);
        int parsedValue = decode_fixed32_le((const uint8_t*) value.data());
        ASSERT_EQ(data[parsedKey].second, parsedValue);
        iter->prev();
    }
}

TEST(DataPageTest, emptyPage) {
    DataPageBuilder builder;
    const Slice& page = builder.finish();
    ASSERT_EQ(0, builder.getRecordNum());
    DataPageReader reader(page);
    ASSERT_EQ(true, reader.checkCRC32C());
    ASSERT_EQ(PageType::kDataPage, reader.getPageType());
    Iterator* iter = reader.newIterator(new IntegerComparator());
    iter->seekToFirst();
    ASSERT_FALSE(iter->valid());
    iter->seekToLast();
    ASSERT_FALSE(iter->valid());
    int target = 200;
    Slice targetSlice((const char*) &target, sizeof(int));
    iter->seek(targetSlice);
    ASSERT_FALSE(iter->valid());
}

TEST(DataPageTest, bigPage) {
    DataPageBuilder builder;
    std::unique_ptr<char[]> buffer(new char[16 * 1024]);
    Slice key("key1");
    Slice value(buffer.get(), 16 * 1024);
    builder.add(key, value);
    ASSERT_GT(builder.estimateSize(), builder.pageSize());
    ASSERT_EQ(1, builder.getRecordNum());
    const Slice& page = builder.finish();
    DataPageReader reader(page);
    ASSERT_EQ(true, reader.checkCRC32C());
    ASSERT_EQ(PageType::kDataPage, reader.getPageType());
    Iterator* iter = reader.newIterator(createLiteLsmDefaultComparator());
    iter->seekToFirst();
    ASSERT_TRUE(iter->valid());
    ASSERT_EQ(key, iter->key());
    ASSERT_EQ(value, iter->value());
    iter->seekToLast();
    ASSERT_TRUE(iter->valid());
    ASSERT_EQ(key, iter->key());
    ASSERT_EQ(value, iter->value());
    iter->seek(key);
    ASSERT_TRUE(iter->valid());
    ASSERT_EQ(key, iter->key());
    ASSERT_EQ(value, iter->value());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm