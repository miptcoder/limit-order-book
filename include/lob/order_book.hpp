#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

namespace lob {
  using Price = uint64_t;
  using Id = uint64_t;
  using Volume = int32_t;

  enum class Side : uint8_t { Buy, Sell };

  struct Order {
    Id id;
    Side side;
    Price price;
    Volume vol;
  };

  struct LvlPrice {
    Volume vol_sum = 0;
    std::list<Order> queue_order;
  };

  class OrderBook {
  public:
    bool add(const Order& order) {
      if (index_.contains(order.id) || order.vol == 0) {
        return false;
      }

      if (order.side == Side::Buy) {
        auto& lvl = bids_[order.price];
        lvl.queue_order.push_back(order);
        lvl.vol_sum += order.vol;
        auto it = --lvl.queue_order.end();
        index_.insert({order.id, it});
      } else {
        auto& lvl = asks_[order.price];
        lvl.queue_order.push_back(order);
        lvl.vol_sum += order.vol;
        auto it = --lvl.queue_order.end();
        index_.insert({order.id, it});
      }
      return true;
    }

    bool cancel(Id id) {
      auto idx_it = index_.find(id);
      if (idx_it == index_.end()) {
        return false;
      }

      auto it = idx_it->second;
      Side sd = it->side;
      Price pr = it->price;
      Volume v = it->vol;

      if (sd == Side::Buy) {
        auto lvl_it = bids_.find(pr);
        LvlPrice& lvl = lvl_it->second;
        lvl.vol_sum -= v;

        lvl.queue_order.erase(it);

        if (lvl.queue_order.empty()) {
          bids_.erase(lvl_it);
        }

      } else {
        auto lvl_it = asks_.find(pr);
        LvlPrice& lvl = lvl_it->second;
        lvl.vol_sum -= v;
        lvl.queue_order.erase(it);

        if (lvl.queue_order.empty()) {
          asks_.erase(lvl_it);
        }
      }

      index_.erase(idx_it);

      return true;
    }

    std::optional<Price> bestBid() const {
      if (bids_.empty()) {
        return std::nullopt;
      }

      auto it_ = bids_.begin();
      return it_->first;
    }

    std::optional<Price> bestAsk() const {
      if (asks_.empty()) {
        return std::nullopt;
      }

      auto it_ = asks_.begin();
      return it_->first;
    }

    Volume volumeAt(Side side, Price price) const {
      if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) {
          return 0;
        }

        return it->second.vol_sum;
      }

      auto it = asks_.find(price);
      if (it == asks_.end()) {
        return 0;
      }

      return it->second.vol_sum;
    }

    size_t orderCount() const { return index_.size(); }

  private:
    std::map<Price, LvlPrice, std::greater<Price>> bids_;
    std::map<Price, LvlPrice, std::less<Price>> asks_;

    std::unordered_map<Id, std::list<Order>::iterator> index_;
  };
}  // namespace lob