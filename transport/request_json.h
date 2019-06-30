#pragma once

#include "json.h"
#include "transport_manager.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Json {

class Request
{
public:
  enum class Type
  {
    UNDEFINED,
    ADD_BUS,
    ADD_STOP,
    GET_BUS,
    GET_STOP
  };

  Request(Type type)
    : type_(type)
  {}
  virtual ~Request() = default;

  virtual void Parse(const Node& node) = 0;
  Type GetType() const { return type_; }

private:
  Type type_ = Type::UNDEFINED;
};

class ModifyRequest : public Request
{
public:
  using Request::Request;

  virtual void Process(TransportManager& tm) const = 0;
};

class ReadRequest : public Request
{
public:
  using Request::Request;

  virtual Node Process(const TransportManager& tm) const = 0;
  int GetID() const { return id_; }

protected:
  int id_ = 0;
};

using ModifyRequestPtr = std::unique_ptr<ModifyRequest>;
ModifyRequestPtr
ParseModifyRequest(const Node& node);

using ReadRequestPtr = std::unique_ptr<ReadRequest>;
ReadRequestPtr
ParseReadRequest(const Node& node);

struct RequestsHandler
{
public:
  void Parse(const Node& node);
  Node Process() const;

private:
  std::vector<ReadRequestPtr> read_requests_;
  std::vector<ModifyRequestPtr> modify_requests_;
};

class AddBusRequest : public ModifyRequest
{
public:
  AddBusRequest()
    : ModifyRequest(Type::ADD_BUS)
  {}

  void Parse(const Node& node) override;
  void Process(TransportManager& tm) const override;

  const TransportManager::BusId& bus() const { return bus_; }
  const std::vector<TransportManager::StopId>& stops() const { return stops_; }

private:
  TransportManager::BusId bus_;
  std::vector<TransportManager::StopId> stops_;
  
  bool is_roundtrip_ = true;
};

class AddStopRequest : public ModifyRequest
{
public:
  AddStopRequest()
    : ModifyRequest(Type::ADD_STOP)
  {}

  void Parse(const Node& node) override;
  void Process(TransportManager& tm) const override;

  double latitude() const { return latitude_; }
  double longitude() const { return longitude_; }
  const TransportManager::StopId& stop() const { return stop_name_; }
  const TransportManager::DistanceTableRecord& record() const
  {
    return record_;
  }

private:
  TransportManager::StopId stop_name_;
  TransportManager::DistanceTableRecord record_;
  double latitude_ = 0.;
  double longitude_ = 0.;
};

class GetBusRequest : public ReadRequest
{
public:
  GetBusRequest()
    : ReadRequest(Type::GET_BUS)
  {}

  void Parse(const Node& node) override;
  Node Process(const TransportManager& tm) const override;

  const TransportManager::BusId& bus() const { return bus_; }

private:
  TransportManager::BusId bus_;
};

class GetStopRequest : public ReadRequest
{
public:
  GetStopRequest()
    : ReadRequest(Type::GET_STOP)
  {}

  void Parse(const Node& node) override;
  Node Process(const TransportManager& tm) const override;

  const TransportManager::StopId& stop() const { return stop_; }

private:
  TransportManager::StopId stop_;
};
}
