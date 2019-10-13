#include "yellow_pages.h"

#include <algorithm>
#include <string_view>
#include <tuple>
#include <vector>

namespace YellowPages {

void
MergeSingular(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers)
{
  const auto* field_desc = company.GetDescriptor()->FindFieldByName(field_name);
  if (!field_desc) {
    throw std::invalid_argument("wrong field name");
  }

  uint32_t max_priority = 0;
  const Company* max_priority_company = nullptr;
  for (const auto& signal : signals) {
    if (signal.has_company()) {
      const Company& signal_company = signal.company();

      const auto* reflection = signal_company.GetReflection();
      if (reflection && reflection->HasField(signal_company, field_desc)) {
        const auto priority = providers.at(signal.provider_id()).priority();
        if (priority >= max_priority) {
          max_priority_company = &signal_company;
          max_priority = priority;
        }
      }
    }
  }

  if (max_priority_company) {
    const auto* company_reflection = company.GetReflection();
    const auto* priority_company_reflection = max_priority_company->GetReflection();

    const auto& priority_message = priority_company_reflection->GetMessage(*max_priority_company, field_desc);
    company_reflection->MutableMessage(&company, field_desc)->CopyFrom(priority_message);
  }
}

void
MergeRepeated(Company& company, const std::string& field_name, const Signals& signals, const Providers& providers)
{

  const auto* field_desc = company.GetDescriptor()->FindFieldByName(field_name);
  if (!field_desc) {
    throw std::invalid_argument("wrong field name");
  }

  uint32_t max_priority = 0;
  std::vector<const Company*> max_priority_companies;
  for (const auto& signal : signals) {
    if (signal.has_company()) {
      const Company& signal_company = signal.company();

      const auto* reflection = signal_company.GetReflection();
      if (reflection->FieldSize(signal_company, field_desc) > 0) {
        const auto priority = providers.at(signal.provider_id()).priority();
        if (priority > max_priority) {
          max_priority_companies.clear();
          max_priority = priority;
        }
        if (priority == max_priority) {
          max_priority_companies.push_back(&signal.company());
        }
      }
    }
  }

  std::unordered_set<std::string> added_messages;

  const auto* company_reflection = company.GetReflection();
  for (const auto* priority_company : max_priority_companies) {
    const auto* priority_company_reflection = priority_company->GetReflection();
    for (int i = 0; i < priority_company_reflection->FieldSize(*priority_company, field_desc); ++i) {
      const auto& priority_message = priority_company_reflection->GetRepeatedMessage(*priority_company, field_desc, i);
      if (added_messages.insert(priority_message.SerializeAsString()).second) {
        company_reflection->AddMessage(&company, field_desc)->CopyFrom(priority_message);
      }
    }
  }
}

Company
Merge(const Signals& signals, const Providers& providers)
{
  Company merged_company;

  MergeSingular(merged_company, "address", signals, providers);
  MergeSingular(merged_company, "working_time", signals, providers);
  MergeRepeated(merged_company, "names", signals, providers);
  MergeRepeated(merged_company, "phones", signals, providers);
  MergeRepeated(merged_company, "urls", signals, providers);

  return merged_company;
}

}
