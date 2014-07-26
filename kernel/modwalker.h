/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *  
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef MODWALKER_H
#define MODWALKER_H

#include "kernel/sigtools.h"
#include "kernel/celltypes.h"

struct ModWalker
{
	struct PortBit
	{
		RTLIL::Cell *cell;
		RTLIL::IdString port;
		int offset;

		bool operator<(const PortBit &other) const {
			if (cell != other.cell)
				return cell < other.cell;
			if (port != other.port)
				return port < other.port;
			return offset < other.offset;
		}
	};

	RTLIL::Design *design;
	RTLIL::Module *module;

	CellTypes ct;
	SigMap sigmap;

	std::map<RTLIL::SigBit, std::set<PortBit>> signal_drivers;
	std::map<RTLIL::SigBit, std::set<PortBit>> signal_consumers;
	std::set<RTLIL::SigBit> signal_inputs, signal_outputs;

	std::map<RTLIL::Cell*, std::set<RTLIL::SigBit>> cell_outputs, cell_inputs;

	void add_wire(RTLIL::Wire *wire)
	{
		if (wire->port_input) {
			std::vector<RTLIL::SigBit> bits = sigmap(wire);
			for (auto bit : bits)
				if (bit.wire != NULL)
					signal_inputs.insert(bit);
		}

		if (wire->port_output) {
			std::vector<RTLIL::SigBit> bits = sigmap(wire);
			for (auto bit : bits)
				if (bit.wire != NULL)
					signal_outputs.insert(bit);
		}
	}

	void add_cell_port(RTLIL::Cell *cell, RTLIL::IdString port, std::vector<RTLIL::SigBit> bits, bool is_output, bool is_input)
	{
		for (int i = 0; i < int(bits.size()); i++)
			if (bits[i].wire != NULL) {
				PortBit pbit = { cell, port, i };
				if (is_output) {
					signal_drivers[bits[i]].insert(pbit);
					cell_outputs[cell].insert(bits[i]);
				}
				if (is_input) {
					signal_consumers[bits[i]].insert(pbit);
					cell_inputs[cell].insert(bits[i]);
				}
			}
	}

	void add_cell(RTLIL::Cell *cell)
	{
		if (ct.cell_known(cell->type)) {
			for (auto &conn : cell->connections())
				add_cell_port(cell, conn.first, sigmap(conn.second),
						ct.cell_output(cell->type, conn.first),
						ct.cell_input(cell->type, conn.first));
		} else {
			for (auto &conn : cell->connections())
				add_cell_port(cell, conn.first, sigmap(conn.second), true, true);
		}
	}

	ModWalker() : design(NULL), module(NULL)
	{
	}

	ModWalker(RTLIL::Design *design, RTLIL::Module *module, CellTypes *filter_ct = NULL)
	{
		setup(design, module, filter_ct);
	}

	void setup(RTLIL::Design *design, RTLIL::Module *module, CellTypes *filter_ct = NULL)
	{
		this->design = design;
		this->module = module;

		ct.clear();
		ct.setup(design);
		sigmap.set(module);

		signal_drivers.clear();
		signal_consumers.clear();
		signal_inputs.clear();
		signal_outputs.clear();

		for (auto &it : module->wires_)
			add_wire(it.second);
		for (auto &it : module->cells)
			if (filter_ct == NULL || filter_ct->cell_known(it.second->type))
				add_cell(it.second);
	}

	// get_* methods -- single RTLIL::SigBit

	template<typename T>
	inline bool get_drivers(std::set<PortBit> &result, RTLIL::SigBit bit) const
	{
		bool found = false;
		if (signal_drivers.count(bit)) {
			const std::set<PortBit> &r = signal_drivers.at(bit);
			result.insert(r.begin(), r.end());
			found = true;
		}
		return found;
	}

	template<typename T>
	inline bool get_consumers(std::set<PortBit> &result, RTLIL::SigBit bit) const
	{
		bool found = false;
		if (signal_consumers.count(bit)) {
			const std::set<PortBit> &r = signal_consumers.at(bit);
			result.insert(r.begin(), r.end());
			found = true;
		}
		return found;
	}

	template<typename T>
	inline bool get_inputs(std::set<RTLIL::SigBit> &result, RTLIL::SigBit bit) const
	{
		bool found = false;
		if (signal_inputs.count(bit))
			result.insert(bit), found = true;
		return found;
	}

	template<typename T>
	inline bool get_outputs(std::set<RTLIL::SigBit> &result, RTLIL::SigBit bit) const
	{
		bool found = false;
		if (signal_outputs.count(bit))
			result.insert(bit), found = true;
		return found;
	}

	// get_* methods -- container of RTLIL::SigBit's (always by reference)

	template<typename T>
	inline bool get_drivers(std::set<PortBit> &result, const T &bits) const
	{
		bool found = false;
		for (RTLIL::SigBit bit : bits)
			if (signal_drivers.count(bit)) {
				const std::set<PortBit> &r = signal_drivers.at(bit);
				result.insert(r.begin(), r.end());
				found = true;
			}
		return found;
	}

	template<typename T>
	inline bool get_consumers(std::set<PortBit> &result, const T &bits) const
	{
		bool found = false;
		for (RTLIL::SigBit bit : bits)
			if (signal_consumers.count(bit)) {
				const std::set<PortBit> &r = signal_consumers.at(bit);
				result.insert(r.begin(), r.end());
				found = true;
			}
		return found;
	}

	template<typename T>
	inline bool get_inputs(std::set<RTLIL::SigBit> &result, const T &bits) const
	{
		bool found = false;
		for (RTLIL::SigBit bit : bits)
			if (signal_inputs.count(bit))
				result.insert(bit), found = true;
		return found;
	}

	template<typename T>
	inline bool get_outputs(std::set<RTLIL::SigBit> &result, const T &bits) const
	{
		bool found = false;
		for (RTLIL::SigBit bit : bits)
			if (signal_outputs.count(bit))
				result.insert(bit), found = true;
		return found;
	}

	// get_* methods -- call by RTLIL::SigSpec (always by value)

	bool get_drivers(std::set<PortBit> &result, RTLIL::SigSpec signal) const
	{
		std::vector<RTLIL::SigBit> bits = sigmap(signal);
		return get_drivers(result, bits);
	}

	bool get_consumers(std::set<PortBit> &result, RTLIL::SigSpec signal) const
	{
		std::vector<RTLIL::SigBit> bits = sigmap(signal);
		return get_consumers(result, bits);
	}

	bool get_inputs(std::set<RTLIL::SigBit> &result, RTLIL::SigSpec signal) const
	{
		std::vector<RTLIL::SigBit> bits = sigmap(signal);
		return get_inputs(result, bits);
	}

	bool get_outputs(std::set<RTLIL::SigBit> &result, RTLIL::SigSpec signal) const
	{
		std::vector<RTLIL::SigBit> bits = sigmap(signal);
		return get_outputs(result, bits);
	}

	// has_* methods -- call by reference

	template<typename T>
	inline bool has_drivers(const T &sig) const {
		std::set<PortBit> result;
		return get_drivers(result, sig);
	}

	template<typename T>
	inline bool has_consumers(const T &sig) const {
		std::set<PortBit> result;
		return get_consumers(result, sig);
	}

	template<typename T>
	inline bool has_inputs(const T &sig) const {
		std::set<RTLIL::SigBit> result;
		return get_inputs(result, sig);
	}

	template<typename T>
	inline bool has_outputs(const T &sig) const {
		std::set<RTLIL::SigBit> result;
		return get_outputs(result, sig);
	}

	// has_* methods -- call by value

	inline bool has_drivers(RTLIL::SigSpec sig) const {
		std::set<PortBit> result;
		return get_drivers(result, sig);
	}

	inline bool has_consumers(RTLIL::SigSpec sig) const {
		std::set<PortBit> result;
		return get_consumers(result, sig);
	}

	inline bool has_inputs(RTLIL::SigSpec sig) const {
		std::set<RTLIL::SigBit> result;
		return get_inputs(result, sig);
	}

	inline bool has_outputs(RTLIL::SigSpec sig) const {
		std::set<RTLIL::SigBit> result;
		return get_outputs(result, sig);
	}
};

#endif
