#include "common.h"
#include "shard.h"
#include "logger.h"
#include <map>
#include <string>

using namespace std;

mutex kv_store_mutex;

void node(const int &rank) {
    auto& logger = Logger::getInstance(rank);
    // logger.setLogFile("node" + to_string(rank) + ".log");
    logger.info("Node " + to_string(rank) + " started...");


    map<int, string> kv_store;
    map<int, bool> prepared_keys;

    while (true) {
        auto [Type, key, value, recPhase] = ShardRequest::receive_shard_request(0, NODE_REQUEST, MPI_COMM_WORLD);

        const auto phase = selectPhase(recPhase);
        const auto type = selectType(Type);

        ShardResponse shard_response{};

        if (type == READ) {
            lock_guard lock(kv_store_mutex);
            if (kv_store.contains(key)) {
                shard_response = {true, kv_store[key]};
                logger.debug("READ success for key: " + to_string(key));
            } else {
                shard_response = {false, "Key not found"};
                logger.debug("READ failed - key not found: " + to_string(key));
            }

            ShardResponse::send_shard_response(shard_response, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        if (phase == PREPARE) {
            lock_guard lock(kv_store_mutex);
            bool can_proceed = false;

            if (prepared_keys[key]) {
                logger.warning("Key " + to_string(key) + " already in PREPARE state");
                ShardResponse::send_shard_response({false, "Already in PREPARE state"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                continue;
            }
            switch (type) {
                case CREATE:
                    can_proceed = !kv_store.contains(key);
                break;
                case UPDATE:
                case DELETE:
                    can_proceed = kv_store.contains(key);
                break;
                default: ;
            }

            if (can_proceed) {
                prepared_keys[key] = true;
                logger.info("PREPARE success for key: " + to_string(key));
                ShardResponse::send_shard_response({true, "PREPARE success"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            } else {
                logger.warning("PREPARE failed for key: " + to_string(key));
                ShardResponse::send_shard_response({false, "PREPARE failed"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            }
            continue;
        }

        if (phase == COMMIT) {
            lock_guard lock(kv_store_mutex);
            if (!prepared_keys[key]) {
                logger.error("COMMIT failed - key not prepared: " + to_string(key));
                ShardResponse::send_shard_response({false, "Not prepared"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                continue;
            }

            bool success = true;
            string msg;

            switch (type) {
                case CREATE:
                case UPDATE:
                    kv_store[key] = value;
                    msg = value;
                    logger.info("COMMIT success - " + string(type == CREATE ? "Created" : "Updated") +
                               " key: " + to_string(key) + " with value: " + value);
                    break;
                case DELETE:
                    kv_store.erase(key);
                    msg = "Deleted";
                    logger.info("COMMIT success - Deleted key: " + to_string(key));
                    break;
                default:
                    success = false;
                    msg = "Invalid operation";
                    logger.error("COMMIT failed - Invalid operation for key: " + to_string(key));
            }

            prepared_keys.erase(key);
            ShardResponse::send_shard_response({success, msg}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        if (phase == ROLLBACK) {
            prepared_keys.erase(key);
            logger.info("ROLLBACK executed for key: " + to_string(key));
            ShardResponse::send_shard_response({false, "Rolled back"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        logger.error("Invalid phase received for key: " + to_string(key));
        ShardResponse::send_shard_response({false, "Invalid phase"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
    }
}
