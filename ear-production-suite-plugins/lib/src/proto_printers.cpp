#include "proto_printers.hpp"

std::ostream& google::protobuf::operator<<(
    std::ostream& os, const google::protobuf::Message& message) {
    std::string jsonMessage;
    using namespace google::protobuf::util;
    JsonPrintOptions options;
    options.always_print_fields_with_no_presence = true;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    MessageToJsonString(message, &jsonMessage, options);
    os << jsonMessage;
    return os;
}
