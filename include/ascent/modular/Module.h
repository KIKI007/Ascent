// Copyright (c) 2016-2019 Anyar, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "ascent/direct/State.h"

#include <memory>
#include <vector>

namespace asc
{
   template <class value_t>
   struct Propagator
   {
      Propagator() = default;
      Propagator(const Propagator&) = default;
      Propagator(Propagator&&) = default;
      Propagator& operator=(const Propagator&) = default;
      Propagator& operator=(Propagator&&) = default;
      virtual ~Propagator() {}

      virtual void operator()(State&, const double) = 0; // inputs: states, dt (time step)

      size_t pass{};
   };

   enum struct Phase
   {
      Link,
      Init,
      Update,
      Postprop,
      Postcalc
   };

   struct Module
   {
      Module() = default;
      Module(const Module&) = default;
      Module(Module&&) = default;
      Module& operator=(const Module&) = default;
      Module& operator=(Module&&) = default;
      virtual ~Module() = default;

      Phase* phase{};

      std::vector<State> states;

      template <class x_t, class xd_t>
      void make_state(x_t& x, xd_t& xd)
      {
         states.emplace_back(x, xd);
      }

      template <class x_t, class xd_t>
      void make_states(x_t& x, xd_t& xd)
      {
         const size_t n = x.size();
         for (size_t i = 0; i < n; ++i)
         {
            states.emplace_back(x[i], xd[i]);
         }
      }

      template <class data_t>
      void make_states(data_t* x, data_t* xd, const size_t n)
      {
         for (size_t i = 0; i < n; ++i)
         {
            states.emplace_back(x[i], xd[i]);
         }
      }

      template <class states_t>
      void add_states(states_t& ext_states)
      {
         for (auto& state : states)
         {
            ext_states.emplace_back(state);
         }
      }

      virtual void link() {} // linking modules
      virtual void init() {} // initialization
      virtual void operator()() {} // derivative updates
      virtual void propagate(Propagator<double>& propagator, const double dt)
      {
         for (auto& state : states)
         {
            propagator(state, dt);
         }
      }
      virtual void postprop() {} // post propagation calculations (every substep)
      virtual void postcalc() {} // post integration calculations (every full step)

      bool init_called = false;
   };

   template <class modules_t>
   inline void init(modules_t& blocks)
   {
      for (auto& block : blocks)
      {
         if (!block->init_called)
         {
            block->init();
            block->init_called = true;
         }   
      }
   }

   template <class modules_t>
   inline void update(modules_t& blocks)
   {
      for (auto& block : blocks)
      {
         block->operator()();
      }
   }

   template <class modules_t>
   inline void postcalc(modules_t& blocks)
   {
      for (auto& block : blocks)
      {
         block->postcalc();
      }
   }

   template <class states_t>
   inline void add_states(states_t& states, std::vector<std::shared_ptr<Module>>& blocks)
   {
      for (auto& block : blocks)
      {
         auto& m_states = block->states;
         for (auto& state : m_states)
         {
            states.emplace_back(state);
         }
      }
   }

   template <class states_t>
   inline void add_states(states_t& states, Module& block)
   {
      for (auto& state : block.states)
      {
         states.emplace_back(state);
      }
   }

   template <class states_t>
   inline void add_states(states_t& states, std::shared_ptr<Module>& block)
   {
      for (auto& state : block->states)
      {
         states.emplace_back(state);
      }
   }
}