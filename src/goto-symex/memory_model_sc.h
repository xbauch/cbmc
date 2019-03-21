/*******************************************************************\

Module: Memory models for partial order concurrency

Author: Michael Tautschnig, michael.tautschnig@cs.ox.ac.uk

\*******************************************************************/

/// \file
/// Memory models for partial order concurrency

#ifndef CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H
#define CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H

#include "memory_model.h"

class memory_model_sct:public memory_model_baset
{
public:
  explicit memory_model_sct(const namespacet &_ns):
    memory_model_baset(_ns)
  {
  }

  virtual void operator()(symex_target_equationt &equation);

protected:
  virtual exprt before(event_it e1, event_it e2);
  virtual bool program_order_is_relaxed(
    partial_order_concurrencyt::event_it e1,
    partial_order_concurrencyt::event_it e2) const;

  /// Order concurrency-relevant events within each thread and match spawn event
  ///   to its target thread
  /// \param equation: the symex equation containing the events in order
  /// \param per_thread_map: thread id to relevant events
  /// \param spawning_map : spawn event to spawned thread id
  void populate_thread_maps(
    const symex_target_equationt &equation,
    per_thread_mapt &per_thread_map,
    spawning_mapt &spawning_map) const;

  /// Add two sets of constrains:
  /// 1. Each spawn event happened before every event of the spawned thread
  /// 2. Every two subsequent events
  /// \param equation:
  void program_order(symex_target_equationt &equation);
  void from_read(symex_target_equationt &equation);
  void write_serialization_external(symex_target_equationt &equation);
};

#endif // CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H
