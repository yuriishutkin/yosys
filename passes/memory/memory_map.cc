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

#include "kernel/register.h"
#include "kernel/log.h"
#include <sstream>
#include <set>
#include <stdlib.h>
#include <assert.h>

static std::string genid(std::string name, std::string token1 = "", int i = -1, std::string token2 = "", int j = -1, std::string token3 = "", int k = -1, std::string token4 = "")
{
	std::stringstream sstr;
	sstr << "$memory" << name << token1;
	
	if (i >= 0)
		sstr << "[" << i << "]";

	sstr << token2;

	if (j >= 0)
		sstr << "[" << j << "]";

	sstr << token3;

	if (k >= 0)
		sstr << "[" << k << "]";

	sstr << token4 << "$" << (RTLIL::autoidx++);
	return sstr.str();
}

static void handle_cell(RTLIL::Module *module, RTLIL::Cell *cell)
{
	std::set<int> static_ports;
	std::map<int, RTLIL::SigSpec> static_cells_map;
	int mem_size = cell->parameters["\\SIZE"].as_int();
	int mem_width = cell->parameters["\\WIDTH"].as_int();
	int mem_offset = cell->parameters["\\OFFSET"].as_int();
	int mem_abits = cell->parameters["\\ABITS"].as_int();

	// delete unused memory cell
	if (cell->parameters["\\RD_PORTS"].as_int() == 0 && cell->parameters["\\WR_PORTS"].as_int() == 0) {
		module->remove(cell);
		return;
	}

	// all write ports must share the same clock
	RTLIL::SigSpec clocks = cell->get("\\WR_CLK");
	RTLIL::Const clocks_pol = cell->parameters["\\WR_CLK_POLARITY"];
	RTLIL::Const clocks_en = cell->parameters["\\WR_CLK_ENABLE"];
	RTLIL::SigSpec refclock;
	RTLIL::State refclock_pol = RTLIL::State::Sx;
	for (int i = 0; i < clocks.size(); i++) {
		RTLIL::SigSpec wr_en = cell->get("\\WR_EN").extract(i * mem_width, mem_width);
		if (wr_en.is_fully_const() && !wr_en.as_bool()) {
			static_ports.insert(i);
			continue;
		}
		if (clocks_en.bits[i] != RTLIL::State::S1) {
			RTLIL::SigSpec wr_addr = cell->get("\\WR_ADDR").extract(i*mem_abits, mem_abits);
			RTLIL::SigSpec wr_data = cell->get("\\WR_DATA").extract(i*mem_width, mem_width);
			if (wr_addr.is_fully_const()) {
				// FIXME: Actually we should check for wr_en.is_fully_const() also and
				// create a $adff cell with this ports wr_en input as reset pin when wr_en
				// is not a simple static 1.
				static_cells_map[wr_addr.as_int()] = wr_data;
				static_ports.insert(i);
				continue;
			}
			log("Not mapping memory cell %s in module %s (write port %d has no clock).\n",
					cell->name.c_str(), module->name.c_str(), i);
			return;
		}
		if (refclock.size() == 0) {
			refclock = clocks.extract(i, 1);
			refclock_pol = clocks_pol.bits[i];
		}
		if (clocks.extract(i, 1) != refclock || clocks_pol.bits[i] != refclock_pol) {
			log("Not mapping memory cell %s in module %s (write clock %d is incompatible with other clocks).\n",
					cell->name.c_str(), module->name.c_str(), i);
			return;
		}
	}

	log("Mapping memory cell %s in module %s:\n", cell->name.c_str(), module->name.c_str());

	std::vector<RTLIL::SigSpec> data_reg_in;
	std::vector<RTLIL::SigSpec> data_reg_out;

	int count_static = 0;

	for (int i = 0; i < mem_size; i++)
	{
		if (static_cells_map.count(i) > 0)
		{
			data_reg_in.push_back(RTLIL::SigSpec(RTLIL::State::Sz, mem_width));
			data_reg_out.push_back(static_cells_map[i]);
			count_static++;
		}
		else
		{
			RTLIL::Cell *c = module->addCell(genid(cell->name, "", i), "$dff");
			c->parameters["\\WIDTH"] = cell->parameters["\\WIDTH"];
			if (clocks_pol.bits.size() > 0) {
				c->parameters["\\CLK_POLARITY"] = RTLIL::Const(clocks_pol.bits[0]);
				c->set("\\CLK", clocks.extract(0, 1));
			} else {
				c->parameters["\\CLK_POLARITY"] = RTLIL::Const(RTLIL::State::S1);
				c->set("\\CLK", RTLIL::SigSpec(RTLIL::State::S0));
			}

			RTLIL::Wire *w_in = module->addWire(genid(cell->name, "", i, "$d"), mem_width);
			data_reg_in.push_back(RTLIL::SigSpec(w_in));
			c->set("\\D", data_reg_in.back());

			std::string w_out_name = stringf("%s[%d]", cell->parameters["\\MEMID"].decode_string().c_str(), i);
			if (module->wires_.count(w_out_name) > 0)
				w_out_name = genid(cell->name, "", i, "$q");

			RTLIL::Wire *w_out = module->addWire(w_out_name, mem_width);
			w_out->start_offset = mem_offset;

			data_reg_out.push_back(RTLIL::SigSpec(w_out));
			c->set("\\Q", data_reg_out.back());
		}
	}

	log("  created %d $dff cells and %d static cells of width %d.\n", mem_size-count_static, count_static, mem_width);

	int count_dff = 0, count_mux = 0, count_wrmux = 0;

	for (int i = 0; i < cell->parameters["\\RD_PORTS"].as_int(); i++)
	{
		RTLIL::SigSpec rd_addr = cell->get("\\RD_ADDR").extract(i*mem_abits, mem_abits);

		std::vector<RTLIL::SigSpec> rd_signals;
		rd_signals.push_back(cell->get("\\RD_DATA").extract(i*mem_width, mem_width));

		if (cell->parameters["\\RD_CLK_ENABLE"].bits[i] == RTLIL::State::S1)
		{
			if (cell->parameters["\\RD_TRANSPARENT"].bits[i] == RTLIL::State::S1)
			{
				RTLIL::Cell *c = module->addCell(genid(cell->name, "$rdreg", i), "$dff");
				c->parameters["\\WIDTH"] = RTLIL::Const(mem_abits);
				c->parameters["\\CLK_POLARITY"] = RTLIL::Const(cell->parameters["\\RD_CLK_POLARITY"].bits[i]);
				c->set("\\CLK", cell->get("\\RD_CLK").extract(i, 1));
				c->set("\\D", rd_addr);
				count_dff++;

				RTLIL::Wire *w = module->addWire(genid(cell->name, "$rdreg", i, "$q"), mem_abits);

				c->set("\\Q", RTLIL::SigSpec(w));
				rd_addr = RTLIL::SigSpec(w);
			}
			else
			{
				RTLIL::Cell *c = module->addCell(genid(cell->name, "$rdreg", i), "$dff");
				c->parameters["\\WIDTH"] = cell->parameters["\\WIDTH"];
				c->parameters["\\CLK_POLARITY"] = RTLIL::Const(cell->parameters["\\RD_CLK_POLARITY"].bits[i]);
				c->set("\\CLK", cell->get("\\RD_CLK").extract(i, 1));
				c->set("\\Q", rd_signals.back());
				count_dff++;

				RTLIL::Wire *w = module->addWire(genid(cell->name, "$rdreg", i, "$d"), mem_width);

				rd_signals.clear();
				rd_signals.push_back(RTLIL::SigSpec(w));
				c->set("\\D", rd_signals.back());
			}
		}

		for (int j = 0; j < mem_abits; j++)
		{
			std::vector<RTLIL::SigSpec> next_rd_signals;

			for (size_t k = 0; k < rd_signals.size(); k++)
			{
				RTLIL::Cell *c = module->addCell(genid(cell->name, "$rdmux", i, "", j, "", k), "$mux");
				c->parameters["\\WIDTH"] = cell->parameters["\\WIDTH"];
				c->set("\\Y", rd_signals[k]);
				c->set("\\S", rd_addr.extract(mem_abits-j-1, 1));
				count_mux++;

				c->set("\\A", module->addWire(genid(cell->name, "$rdmux", i, "", j, "", k, "$a"), mem_width));
				c->set("\\B", module->addWire(genid(cell->name, "$rdmux", i, "", j, "", k, "$b"), mem_width));

				next_rd_signals.push_back(c->get("\\A"));
				next_rd_signals.push_back(c->get("\\B"));
			}

			next_rd_signals.swap(rd_signals);
		}

		for (int j = 0; j < mem_size; j++)
			module->connect(RTLIL::SigSig(rd_signals[j], data_reg_out[j]));
	}

	log("  read interface: %d $dff and %d $mux cells.\n", count_dff, count_mux);

	for (int i = 0; i < mem_size; i++)
	{
		if (static_cells_map.count(i) > 0)
			continue;

		RTLIL::SigSpec sig = data_reg_out[i];

		for (int j = 0; j < cell->parameters["\\WR_PORTS"].as_int(); j++)
		{
			RTLIL::SigSpec wr_addr = cell->get("\\WR_ADDR").extract(j*mem_abits, mem_abits);
			RTLIL::SigSpec wr_data = cell->get("\\WR_DATA").extract(j*mem_width, mem_width);
			RTLIL::SigSpec wr_en = cell->get("\\WR_EN").extract(j*mem_width, mem_width);

			RTLIL::Cell *c = module->addCell(genid(cell->name, "$wreq", i, "", j), "$eq");
			c->parameters["\\A_SIGNED"] = RTLIL::Const(0);
			c->parameters["\\B_SIGNED"] = RTLIL::Const(0);
			c->parameters["\\A_WIDTH"] = cell->parameters["\\ABITS"];
			c->parameters["\\B_WIDTH"] = cell->parameters["\\ABITS"];
			c->parameters["\\Y_WIDTH"] = RTLIL::Const(1);
			c->set("\\A", RTLIL::SigSpec(i, mem_abits));
			c->set("\\B", wr_addr);
			count_wrmux++;

			RTLIL::Wire *w_seladdr = module->addWire(genid(cell->name, "$wreq", i, "", j, "$y"));
			c->set("\\Y", w_seladdr);

			int wr_offset = 0;
			while (wr_offset < wr_en.size())
			{
				int wr_width = 1;
				RTLIL::SigSpec wr_bit = wr_en.extract(wr_offset, 1);

				while (wr_offset + wr_width < wr_en.size()) {
					RTLIL::SigSpec next_wr_bit = wr_en.extract(wr_offset + wr_width, 1);
					if (next_wr_bit != wr_bit)
						break;
					wr_width++;
				}

				RTLIL::Wire *w = w_seladdr;

				if (wr_bit != RTLIL::SigSpec(1, 1))
				{
					c = module->addCell(genid(cell->name, "$wren", i, "", j, "", wr_offset), "$and");
					c->parameters["\\A_SIGNED"] = RTLIL::Const(0);
					c->parameters["\\B_SIGNED"] = RTLIL::Const(0);
					c->parameters["\\A_WIDTH"] = RTLIL::Const(1);
					c->parameters["\\B_WIDTH"] = RTLIL::Const(1);
					c->parameters["\\Y_WIDTH"] = RTLIL::Const(1);
					c->set("\\A", w);
					c->set("\\B", wr_bit);

					w = module->addWire(genid(cell->name, "$wren", i, "", j, "", wr_offset, "$y"));
					c->set("\\Y", RTLIL::SigSpec(w));
				}

				c = module->addCell(genid(cell->name, "$wrmux", i, "", j, "", wr_offset), "$mux");
				c->parameters["\\WIDTH"] = wr_width;
				c->set("\\A", sig.extract(wr_offset, wr_width));
				c->set("\\B", wr_data.extract(wr_offset, wr_width));
				c->set("\\S", RTLIL::SigSpec(w));

				w = module->addWire(genid(cell->name, "$wrmux", i, "", j, "", wr_offset, "$y"), wr_width);
				c->set("\\Y", w);

				sig.replace(wr_offset, w);
				wr_offset += wr_width;
			}
		}

		module->connect(RTLIL::SigSig(data_reg_in[i], sig));
	}

	log("  write interface: %d blocks of $eq, $and and $mux cells.\n", count_wrmux);

	module->remove(cell);
}

static void handle_module(RTLIL::Design *design, RTLIL::Module *module)
{
	std::vector<RTLIL::Cell*> cells;
	for (auto &it : module->cells)
		if (it.second->type == "$mem" && design->selected(module, it.second))
			cells.push_back(it.second);
	for (auto cell : cells)
		handle_cell(module, cell);
}

struct MemoryMapPass : public Pass {
	MemoryMapPass() : Pass("memory_map", "translate multiport memories to basic cells") { }
	virtual void help()
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    memory_map [selection]\n");
		log("\n");
		log("This pass converts multiport memory cells as generated by the memory_collect\n");
		log("pass to word-wide DFFs and address decoders.\n");
		log("\n");
	}
	virtual void execute(std::vector<std::string> args, RTLIL::Design *design) {
		log_header("Executing MEMORY_MAP pass (converting $mem cells to logic and flip-flops).\n");
		extra_args(args, 1, design);
		for (auto &mod_it : design->modules)
			if (design->selected(mod_it.second))
				handle_module(design, mod_it.second);
	}
} MemoryMapPass;
 
