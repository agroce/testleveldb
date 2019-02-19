#include "leveldb/db.h"
#include "leveldb/write_batch.h"

#include "Common.hpp"

#include <deepstate/DeepState.hpp>
using namespace deepstate;

bool check_it_valid(leveldb::Iterator *l_it) {
  if (l_it == nullptr) {
    return false;
  }
  if (!l_it->Valid()) {
    return false;
  }
  return true;
}

TEST(LevelDB, Fuzz) {
  rmrf(LEVELDB_LOCATION);
  
  leveldb::DB* l_db;
  leveldb::Iterator *l_it = nullptr;
  leveldb::Options l_options;
  l_options.create_if_missing = true;
  leveldb::Status l_status = leveldb::DB::Open(l_options, LEVELDB_LOCATION, &l_db);
  ASSERT (l_status.ok()) << "Could not create the leveldb test database!";
  leveldb::WriteBatch l_batch;

  for (int n=0; n < TEST_LENGTH; n++) {
    OneOf(
	  [&] {
	    char* key = GetKey();
	    char* value = GetValue();
	    bool synced = DeepState_Bool();	    
	    LOG(TRACE) << "STEP #" << n << ": PUT <" << key << "> <" << value << "> " << synced;

	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;	    
	    }
	    leveldb::Status l_s = l_db->Put(l_write_options, key, value);
	    if (!l_s.ok()) {
	      LOG(TRACE) << "STEP #" << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = GetKey();
	    char* value = GetValue();

	    LOG(TRACE) << "STEP #" << n << ": BATCH PUT <" << key << "> <" << value << ">";
	    
	    l_batch.Put(key, value);
	  },	  
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << "STEP #" << n << ": GET <" << key << ">";

	    std::string l_value;
	    leveldb::Status l_s = l_db->Get(leveldb::ReadOptions(), key, &l_value);
	    if (l_s.ok()) {
	      LOG(TRACE) << n << ": RESULT:" << l_value;
	    } else {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = GetKey();
	    bool synced = DeepState_Bool();
	    LOG(TRACE) << "STEP #" << n << ": DELETE <" << key << "> " << synced;

	    leveldb::WriteOptions l_write_options;
	    if (synced) {
	      l_write_options.sync = true;	    
	    }	    
	    leveldb::Status l_s = l_db->Delete(l_write_options, key);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << "STEP #" << n << ": BATCH DELETE <" << key << ">";
	    
	    l_batch.Delete(key);
	  },	  
	  [&] {
	    bool synced = DeepState_Bool();

	    LOG(TRACE) << "STEP #" << n << ": BATCH WRITE " << synced;

	    leveldb::WriteOptions l_write_options;

	    if (synced) {
	      l_write_options.sync = true;
	    }
	    
	    leveldb::Status l_s = l_db->Write(l_write_options, &l_batch);
	    if (!l_s.ok()) {
	      LOG(TRACE) << n << ": leveldb NOT OK: " << l_s.ToString();
	    }
	  },
	  [&] {
	    LOG(TRACE) << "STEP #" << n << ": BATCH CLEAR";
	    
	    l_batch.Clear();
	  },
	  [&] {
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR CREATE";

	    if (l_it != nullptr) {
	      delete l_it;
	    }
	    
	    l_it = l_db->NewIterator(leveldb::ReadOptions());
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR SEEK <" << key << ">";

	    if (check_it_valid(l_it)) {
	      l_it->Seek(key);
	    }
	  },
	  [&] {
	    char* key = GetKey();
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR SEEKTOLAST";

	    if (check_it_valid(l_it)) {
	      l_it->SeekToLast();
	    }
	  },
	  [&] {
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR GET";

	    if (check_it_valid(l_it)) {
	      std::string l_key = l_it->key().ToString();
	      std::string l_value = l_it->value().ToString();
	    }
	  },
	  [&] {
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR NEXT";

	    if (check_it_valid(l_it)) {
	      l_it->Next();
	    }
	  },
	  [&] {
	    LOG(TRACE) << "STEP #" << n << ": ITERATOR PREV";

	    if (check_it_valid(l_it)) {
	      l_it->Prev();
	    }
	  },
	  [&] {
	    unsigned int write_buffer_size = DeepState_UIntInRange(128, 64 * 1024 * 2048);
	    LOG(TRACE) << "STEP #" << n << ": SET ROCKSDB write_buffer_size " << write_buffer_size;
	  },
	  [&] {
	    unsigned int max_write_buffer_number = DeepState_UIntInRange(2, 10);
	    LOG(TRACE) << "STEP #" << n << ": SET ROCKSDB max_write_buffer_number " << max_write_buffer_number;
	  }
	  );
  }

  if (l_it != nullptr) {
    delete l_it;
  }
  
  delete l_db;
}
