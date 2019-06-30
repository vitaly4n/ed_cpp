#pragma once

#include "transport_manager.h"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct Request
{
  enum class Type
  {
    ADD_BUS,
    ADD_STOP,
    GET_BUS,
    GET_STOP
  };

  Request(Type type);

  virtual ~Request() = default;
  virtual void read(std::string_view data) = 0;

  const Type type_;
};

template<typename T>
struct ReadRequest : public Request
{
  using Request::Request;
  virtual T process(const TransportManager& tm) const = 0;
};

struct WriteRequest : public Request
{
  using Request::Request;
  void read(std::string_view data) override final;

  virtual void read(std::string_view operation, std::string_view operand) = 0;
  virtual void process(TransportManager& tm) const = 0;
};

struct AddBusRequest : public WriteRequest
{
  AddBusRequest();

  bool is_one_way(std::string_view route_desc) const;
  void read(std::string_view operation, std::string_view operand) override;
  void process(TransportManager& tm) const override;

  TransportManager::BusId bus_;
  std::vector<std::string_view> stops_;
  std::string stops_data_;
};

struct AddBusStopRequest : public WriteRequest
{
  AddBusStopRequest();

  void read(std::string_view operation, std::string_view operand) override;
  void process(TransportManager& tm) const override;

  std::string stop_name_;
  double latitude_ = 0.;
  double longitude_ = 0.;
  TransportManager::DistanceTableRecord record_;
};

struct GetBusRequest : public ReadRequest<std::string>
{
  GetBusRequest();

  void read(std::string_view data) override;
  std::string process(const TransportManager& tm) const override;

  TransportManager::BusId bus_;
};

struct GetStopRequest : public ReadRequest<std::string>
{
  GetStopRequest();

  void read(std::string_view data) override;
  std::string process(const TransportManager& tm) const override;

  TransportManager::StopId stop_;
};

using RequestPtr = std::unique_ptr<Request>;

RequestPtr
read_request(std::string_view request_str);

std::vector<RequestPtr>
read_requests(std::istream& is = std::cin);

std::vector<std::string>
process_requests(const std::vector<RequestPtr>& requests);

void
print_responses(const std::vector<std::string>& responses,
                std::ostream& os = std::cout);
