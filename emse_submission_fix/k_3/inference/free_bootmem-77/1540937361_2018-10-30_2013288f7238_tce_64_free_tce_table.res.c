void __init free_tce_table(void *tbl)
{
	unsigned int size;
	if (!tbl)
		return;
	size = table_size_to_number_of_entries(specified_table_size);
	size *= TCE_ENTRY_SIZE;
	memblock_free(__pa(tbl), size);
}