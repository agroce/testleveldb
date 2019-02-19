#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "rocksdb/db.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/options.h"
#include "rocksdb/write_batch.h"

#include "Common.hpp"

#include <deepstate/DeepState.hpp>
using namespace deepstate;

void check_status(int n, leveldb::Status l_s, rocksdb::Status r_s) {
  LOG(TRACE) << n << ": leveldb STATUS: " << l_s.ToString();
  LOG(TRACE) << n << ": rocksdb STATUS: " << r_s.ToString();
  ASSERT((l_s.ok() && r_s.ok()) || ((!l_s.ok()) && (!r_s.ok()))) << "Status mismatch: " <<
    l_s.ToString() << " vs. " << r_s.ToString();
}

bool check_it_valid(leveldb::Iterator *l_it, rocksdb::Iterator *r_it) {
  if ((l_it == nullptr) || (r_it == nullptr)) {
    ASSERT((l_it == nullptr) == (r_it == nullptr)) << "Mismatch in iterator null: " <<
      l_it << " vs. " << r_it;
    return false;
  }
  if ((!l_it->Valid()) || (!r_it->Valid())) {
    ASSERT((l_it->Valid() == r_it->Valid())) << "Mismatch in iterator validity: " <<
      l_it->Valid() << " vs. " << r_it->Valid();
    return false;
  }
  leveldb::Status l_s = l_it->status();
  rocksdb::Status r_s = r_it->status();  
  ASSERT((l_s.ok() && r_s.ok()) || ((!l_s.ok()) && (!r_s.ok()))) <<
    "Iterator status mismatch: " << l_s.ToString() << " vs. " << r_s.ToString();  
  return true;
}

TEST(LevelDB, Fuzz) {
  rmrf(LEVELDB_LOCATION);
  rmrf(ROCKSDB_LOCATION);
  
  leveldb::DB* l_db;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_s = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT(l_s.ok()) << "Could not create the leveldb test database!";

  rocksdb::DB* r_db;
  rocksdb::Options r_options;
  r_options.create_if_missing = true;
  rocksdb::Status r_s = rocksdb::DB::Open(r_options, ROCKSDB_LOCATION, &r_db);
  ASSERT(r_s.ok()) << "Could not create the rocksdb test database!";

  leveldb::WriteBatch l_batch;
  rocksdb::WriteBatch r_batch;

  leveldb::Iterator *l_it = nullptr;
  rocksdb::Iterator *r_it = nullptr;
  
  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = GetKey();
	    char* value = GetValue();
	    bool synced = DeepState_Bool();
	    LOG(TRACE) << n << ": PUT <" << key << "> <" << value << "> " << synced;

	    rocksdb::WriteOptions r_write_options;
	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;
	      r_write_options.sync = true;
	    }

	    leveldb::Status l_s = l_db->Put(l_write_options, key, value);
	    rocksdb::Status r_s = r_db->Put(r_write_options, key, value);
	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    char* key = GetKey();
	    char* value = GetValue();

	    LOG(TRACE) << n << ": BATCH PUT <" << key << "> <" << value << ">";
	    
	    l_batch.Put(key, value);
	    r_batch.Put(key, value);
	  },	  
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << n << ": GET <" << key << ">";

	    std::string l_value;
	    std::string r_value;

	    leveldb::Status l_s = l_db->Get(leveldb::ReadOptions(), key, &l_value);
	    rocksdb::Status r_s = r_db->Get(rocksdb::ReadOptions(), key, &r_value);

	    check_status(n, l_s, r_s);
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT: <" << l_value << ">";
	    }
	    ASSERT_EQ(l_value, r_value) << l_value << " SHOULD EQUAL " << r_value;
	  },
	  [&] {
	    char* key = GetKey();
	    bool synced = DeepState_Bool();
	    LOG(TRACE) << n << ": DELETE <" << key << "> " << synced;

	    leveldb::WriteOptions l_write_options;
	    rocksdb::WriteOptions r_write_options;
	    if (synced) {
	      l_write_options.sync = true;
	      r_write_options.sync = true;	      
	    }
	    
	    leveldb::Status l_s = l_db->Delete(l_write_options, key);
	    rocksdb::Status r_s = r_db->Delete(r_write_options, key);

	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << n << ": BATCH DELETE <" << key << ">";
	    
	    l_batch.Delete(key);
	    r_batch.Delete(key);
	  },	  
	  [&] {
	    bool synced = DeepState_Bool();	    
	    LOG(TRACE) << n << ": BATCH WRITE " << synced;

	    leveldb::WriteOptions l_write_options;
	    rocksdb::WriteOptions r_write_options;

	    if (synced) {
	      l_write_options.sync = true;
	      r_write_options.sync = true;
	    }

	    leveldb::Status l_s = l_db->Write(l_write_options, &l_batch);
	    rocksdb::Status r_s = r_db->Write(r_write_options, &r_batch);
	    check_status(n, l_s, r_s);
	  },
	  [&] {
	    LOG(TRACE) << n << ": BATCH CLEAR";
	    
	    l_batch.Clear();
	    r_batch.Clear();
	  },
	  [&] {
	    LOG(TRACE) << n << ": ITERATOR CREATE";

	    if (l_it != nullptr) {
	      delete l_it;
	      delete r_it;
	    }
	    
	    l_it = l_db->NewIterator(leveldb::ReadOptions());
	    r_it = r_db->NewIterator(rocksdb::ReadOptions());
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << n << ": ITERATOR SEEK <" << key << ">";

	    if (check_it_valid(l_it, r_it)) {
	      l_it->Seek(key);
	      r_it->Seek(key);
	      check_it_valid(l_it, r_it);
	    }
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << n << ": ITERATOR SEEKTOLAST";

	    if (check_it_valid(l_it, r_it)) {
	      l_it->SeekToLast();
	      r_it->SeekToLast();
	      check_it_valid(l_it, r_it);
	    }
	  },
	  [&] {
	    LOG(TRACE) << n << ": ITERATOR GET";

	    if (check_it_valid(l_it, r_it)) {
	      std::string l_key = l_it->key().ToString();
	      std::string r_key = r_it->key().ToString();
	      std::string l_value = l_it->value().ToString();
	      std::string r_value = r_it->value().ToString();
	      ASSERT_EQ(l_key, r_key) << "Mismatch in keys:" <<
		l_key << " vs. " << r_key;
	      ASSERT_EQ(l_key, r_key) << "Mismatch in values:" <<
		l_value << " vs. " << r_value;
	    }
	  },
	  [&] {
	    LOG(TRACE) << n << ": ITERATOR NEXT";

	    if (check_it_valid(l_it, r_it)) {
	      l_it->Next();
	      r_it->Next();
	      check_it_valid(l_it, r_it);
	    }
	  },
	  [&] {
	    LOG(TRACE) << n << ": ITERATOR PREV";

	    if (check_it_valid(l_it, r_it)) {
	      l_it->Prev();
	      r_it->Prev();
	      check_it_valid(l_it, r_it);
	    }
	  },
	  [&] {
	    unsigned int write_buffer_size = DeepState_UIntInRange(128, 64 * 1024 * 2048);
	    LOG(TRACE) << n << ": SET ROCKSDB write_buffer_size " << write_buffer_size;
	    rocksdb::Status r_s = r_db->SetOptions({{"write_buffer_size", std::to_string(write_buffer_size)}});
	    ASSERT(r_s.ok()) << "Failed to set write buffer size!";
	  },
	  [&] {
	    unsigned int max_write_buffer_number = DeepState_UIntInRange(2, 10);
	    LOG(TRACE) << n << ": SET ROCKSDB max_write_buffer_number " << max_write_buffer_number;
	    rocksdb::Status r_s = r_db->SetOptions({{"max_write_buffer_number", std::to_string(max_write_buffer_number)}});
	    ASSERT(r_s.ok()) << "Failed to set max write buffer number!";
	  }	  
	  );
  }

  if (l_it != nullptr) {
    delete l_it;
  }

  if (l_it != nullptr) {
    delete r_it;
  }
  
  delete l_db;
  delete r_db;
}
