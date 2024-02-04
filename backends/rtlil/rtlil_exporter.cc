/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
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
 *  ---
 *
 *  A very simple and straightforward backend for the RTLIL text
 *  representation.
 *
 */

#include "rtlil_exporter.h"
#include "kernel/yosys.h"
#include <errno.h>

USING_YOSYS_NAMESPACE
using namespace RTLIL_EXPORTER;
YOSYS_NAMESPACE_BEGIN


namespace RTLIL_EXPORTER
{

export_attribute_cb export_attribute_cb_f;
export_parameter_cb export_parameter_cb_f;
export_parameter_val_cb export_parameter_val_cb_f;
export_wire_cb export_wire_cb_f;
export_memory_cb export_memory_cb_f;
export_cell_cb export_cell_cb_f;
export_proc_cb export_proc_cb_f;
export_conn_cb export_conn_cb_f;
export_module_cb export_module_start_cb_f;
export_module_cb export_module_end_cb_f;
export_design_cb export_design_cb_f;

std::string module_name(const RTLIL::Module *module) { return module->name.str(); }
std::string idString_str(const RTLIL::IdString& idString) { return idString.str(); }

void export_module(RTLIL::Module *module, RTLIL::Design *design)
{
	int rc = export_module_start_cb_f(module, design);

	if (rc != 0) return;

	for (auto it = module->attributes.begin(); it != module->attributes.end(); ++it) {
		export_attribute_cb_f(module, *it);
	}

	if (!module->avail_parameters.empty()) {
		for (const auto &p : module->avail_parameters) {
			const auto &it = module->parameter_default_values.find(p);
			if (it == module->parameter_default_values.end()) {
				export_parameter_cb_f(module, p);
			} else {
				export_parameter_val_cb_f(module, p, it->second);
			}
		}
	}

	for (auto it : module->wires())
		export_wire_cb_f(module, it);

	for (auto it : module->memories)
		export_memory_cb_f(module, it.second);

	for (auto it : module->cells())
		export_cell_cb_f(module, it);

	for (auto it : module->processes)
		export_proc_cb_f(module, it.second);

	for (auto it = module->connections().begin(); it != module->connections().end(); ++it) {
		export_conn_cb_f(module, it->first, it->second);
	}

	export_module_end_cb_f(module, design);
}

void export_design(RTLIL::Design *design)
{
	export_design_cb_f(design);

	for (auto module : design->modules()) {
		export_module(module, design);
	}
}
} // namespace RTLIL_EXPORTER

YOSYS_NAMESPACE_END
