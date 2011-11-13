#pragma once

struct OperationStatus
{
	enum type { scheduled, started, running, completed, failed };
};