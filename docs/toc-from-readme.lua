-- toc-from-readme.lua
-- Extracts the BulletList under "## Index" in README.md
-- and writes Qt Help <section> XML to TOC_FILE (toc.xml).

local toc_file = os.getenv("TOC_FILE") or "toc.xml"
local toc_lines = {}

-- Recursively write <section> tags for a BulletList
local function write_sections(items, level)
  for _, item in ipairs(items) do
    local blocks = item
    local first = blocks[1]

    if first and (first.t == "Para" or first.t == "Plain") then
      -- This gives "1 Getting Started", "3.1 File", etc.
      local title = pandoc.utils.stringify(first)
      local ref = nil

      -- Use the first link in the paragraph as the target
      for _, inline in ipairs(first.content) do
        if inline.t == "Link" then
          ref = inline.target
          break
        end
      end

      if ref then
        -- Map .md -> .html
        ref = ref:gsub("%.md$", ".html")
        table.insert(
          toc_lines,
          string.format('%s<section title="%s" ref="%s">',
                        string.rep("  ", level),
                        title,
                        ref)
        )
      else
        table.insert(
          toc_lines,
          string.format('%s<section title="%s">',
                        string.rep("  ", level),
                        title)
        )
      end

      -- Look for nested BulletLists in the remaining blocks
      for i = 2, #blocks do
        local b = blocks[i]
        if b.t == "BulletList" then
          write_sections(b.content, level + 1)
        end
      end

      table.insert(
        toc_lines,
        string.rep("  ", level) .. "</section>"
      )
    end
  end
end

function Pandoc(doc)
  local in_index = false

  for i, blk in ipairs(doc.blocks) do
    if blk.t == "Header" and blk.level == 2
       and pandoc.utils.stringify(blk) == "Index" then
      -- Found "## Index"
      in_index = true

    elseif in_index then
      if blk.t == "BulletList" then
        -- This is the list we want: convert whole list and write file
        write_sections(blk.content, 1)
        local f = assert(io.open(toc_file, "w"))
        f:write(table.concat(toc_lines, "\n"))
        f:close()
        break

      elseif blk.t == "Header" then
        -- Next section started and we never saw a list; give up
        break
      end
    end
  end

  return doc
end

