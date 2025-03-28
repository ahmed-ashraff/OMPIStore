#include "../../include/node/node.h"
#include "../../include/common/logger.h"
#include "../../include/common/types.h"
#include "../../include/common/node_request_utils.h"
#include "../../include/common/node_response_utils.h"
#include <map>
#include <string>
#include <fstream>
#include <omp.h>

using namespace std;

mutex kv_store_mutex;


void node(const int &rank) {
    auto &logger = Logger::getInstance();
    logger.info("Node " + to_string(rank) + " started...", rank);

    map<int, string> kv_store;
    map<int, bool> prepared_keys;

    while (true) {
        auto [type, key, value, recPhase] = receive_node_request(0, NODE_REQUEST, MPI_COMM_WORLD);

        NodeResponse node_response{};


        if (type == RequestType::READ) {
            lock_guard lock(kv_store_mutex);
            if (kv_store.contains(key)) {
                node_response = {true, kv_store[key]};
                logger.debug("READ success for key: " + to_string(key), rank);
            } else {
                node_response = {false, "Key not found"};
                logger.debug("READ failed - key not found: " + to_string(key), rank);
            }

            send_node_response(node_response, 0, NODE_RESPONSE, MPI_COMM_WORLD);
        }

        if (recPhase == TwoPC::PREPARE) {
            lock_guard lock(kv_store_mutex);

            if (prepared_keys[key]) {
                logger.warning("Key " + to_string(key) + " already in PREPARE state", rank);
                send_node_response({false, "Already in PREPARE state"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            } else {
                bool can_proceed = false;
                switch (type) {
                    case RequestType::CREATE:
                        can_proceed = !kv_store.contains(key);
                        break;
                    case RequestType::UPDATE:
                    case RequestType::DELETE:
                        can_proceed = kv_store.contains(key);
                        break;
                    default:
                        break;
                }

                if (can_proceed) {
                    prepared_keys[key] = true;
                    logger.info("PREPARE success for key: " + to_string(key), rank);
                    send_node_response({true, "PREPARE success"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                } else {
                    logger.warning("PREPARE failed for key: " + to_string(key), rank);
                    send_node_response({false, "PREPARE failed"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                }
            }
        }

        if (recPhase == TwoPC::COMMIT) {
            lock_guard lock(kv_store_mutex);
            if (!prepared_keys[key]) {
                logger.error("COMMIT failed - key not prepared: " + to_string(key), rank);
                send_node_response({false, "Not prepared"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            } else {
                bool success = true;
                string msg;

                switch (type) {
                    case RequestType::CREATE:
                    case RequestType::UPDATE:
                        kv_store[key] = value;
                        msg = value;
                        logger.info("COMMIT success - " + string(type == RequestType::CREATE ? "Created" : "Updated") +
                                    " key: " + to_string(key) + " with value: " + value, rank);
                        break;
                    case RequestType::DELETE:
                        kv_store.erase(key);
                        msg = "Deleted";
                        logger.info("COMMIT success - Deleted key: " + to_string(key), rank);
                        break;
                    default:
                        success = false;
                        msg = "Invalid operation";
                        logger.error("COMMIT failed - Invalid operation for key: " + to_string(key), rank);
                }

                prepared_keys.erase(key);
                send_node_response({success, msg}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            }
        }

        if (recPhase == TwoPC::ROLLBACK) {
            prepared_keys.erase(key);
            logger.info("ROLLBACK executed for key: " + to_string(key), rank);
            send_node_response({false, "Rolled back"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
        }
    }
}
