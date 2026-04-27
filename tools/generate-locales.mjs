#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';

const require = createRequire(import.meta.url);
const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(scriptDir, '..');
const packageRoot = path.resolve(process.argv[2] || path.join(repoRoot, '.tmp/npm-package'));
const outPath = path.join(repoRoot, 'include/polycpp/moment/detail/generated/locales.hpp');
const cmakeOutPath = path.join(repoRoot, 'cmake/PolycppMomentLocales.cmake');

const moment = require(path.join(packageRoot, 'moment.js'));
const localeDir = path.join(packageRoot, 'locale');
const localeFiles = fs.readdirSync(localeDir)
  .filter((file) => file.endsWith('.js'))
  .sort();

for (const file of localeFiles) {
  require(path.join(localeDir, file));
}

function cppString(value) {
  return `"${String(value)
    .replace(/\\/g, '\\\\')
    .replace(/"/g, '\\"')
    .replace(/\r/g, '\\r')
    .replace(/\n/g, '\\n')
    .replace(/\t/g, '\\t')}"`;
}

function ident(code) {
  return code.replace(/[^A-Za-z0-9_]/g, '_');
}

function macroName(code) {
  return `POLYCPP_MOMENT_LOCALE_${ident(code)}`;
}

function arrayInit(values, indent = '        ') {
  return `{{\n${values.map((value) => `${indent}${cppString(value)}`).join(',\n')}\n${indent.slice(0, -4)}}}`;
}

function momentForMonth(month) {
  return moment.utc([2024, month, 15, 14, 30, 45, 123]);
}

function momentForWeekday(day, hour = 14, sameWeek = true) {
  const base = moment.utc([2024, 0, 7 + day, hour, 30, 45, 123]);
  const reference = sameWeek ? base.clone() : base.clone().subtract(7, 'days');
  return { value: base, reference };
}

function weekdayFormatSample(source) {
  if (!source) return 'dddd';
  if (source === 'dddd HH:mm') return 'dddd HH:mm';
  if (source === '(წინა|შემდეგ)') return 'წინა dddd';
  if (source.includes('[Вв]')) return '[В] dddd';
  if (source.includes('[Ууў]')) return '[У] dddd';
  return 'dddd';
}

function numberRange(start, end) {
  const values = [];
  for (let i = start; i <= end; ++i) values.push(i);
  return values;
}

function ordinalTable(ld, token, values) {
  return values.map((value) => ld.ordinal(value, token));
}

function calendarTable(ld, key) {
  const weeks = [];
  for (const sameWeek of [true, false]) {
    const hours = [];
    for (let hour = 0; hour < 24; ++hour) {
      const days = [];
      for (let day = 0; day < 7; ++day) {
        const { value, reference } = momentForWeekday(day, hour, sameWeek);
        days.push(ld.calendar(key, value, reference));
      }
      hours.push(days);
    }
    weeks.push(hours);
  }
  return weeks;
}

function emitCalendarTable(table) {
  const weekBlocks = table.map((week) => {
    const hourBlocks = week.map((days) => `            ${arrayInit(days, '                ')}`);
    return `        {{\n${hourBlocks.join(',\n')}\n        }}`;
  });
  return `{{\n${weekBlocks.join(',\n')}\n    }}`;
}

function replaceNumberWithMarker(output, number) {
  return String(output).replace(String(number), '%d');
}

function safeRelativeTime(ld, number, withoutSuffix, key, isFuture) {
  try {
    return ld.relativeTime(number, withoutSuffix, key, isFuture);
  } catch {
    const safeNumber = key.length === 2 ? 2 : 1;
    let fallback = ld.relativeTime(safeNumber, withoutSuffix, key, isFuture);
    if (number !== safeNumber) {
      fallback = replaceNumberWithMarker(fallback, safeNumber).replace('%d', String(number));
    }
    return fallback;
  }
}

function relativeOutputs(ld, rt, key, wrapperFunctions) {
  const raw = rt[key];
  const values = numberRange(0, 200);
  const make = (withoutSuffix, isFuture) => values.map((number) => {
    const inner = safeRelativeTime(ld, number, withoutSuffix, key, isFuture);
    if (!wrapperFunctions || withoutSuffix) return inner;
    const wrapper = isFuture ? rt.future : rt.past;
    if (typeof wrapper === 'function') return wrapper(inner);
    return String(wrapper).replace('%s', inner);
  });

  if (typeof raw !== 'function' && !wrapperFunctions) {
    return { kind: 'string', value: String(raw) };
  }

  return {
    kind: 'table',
    noSuffixPast: make(true, false),
    noSuffixFuture: make(true, true),
    withSuffixPast: make(false, false),
    withSuffixFuture: make(false, true),
  };
}

function emitRelativeTable(table) {
  const rows = [
    ['noSuffixPast', table.noSuffixPast],
    ['noSuffixFuture', table.noSuffixFuture],
    ['withSuffixPast', table.withSuffixPast],
    ['withSuffixFuture', table.withSuffixFuture],
  ];
  return rows.map(([name, values]) => (
`        static const std::array<const char*, 201> ${name} = ${arrayInit(values, '            ')};`
  )).join('\n');
}

function meridiemSegments(ld) {
  const rows = [];
  let current = null;
  for (let minuteOfDay = 0; minuteOfDay < 24 * 60; ++minuteOfDay) {
    const hour = Math.floor(minuteOfDay / 60);
    const minute = minuteOfDay % 60;
    const upper = ld.meridiem(hour, minute, false);
    const lower = ld.meridiem(hour, minute, true);
    if (current && current.upper === upper && current.lower === lower && current.end + 1 === minuteOfDay) {
      current.end = minuteOfDay;
    } else {
      current = { start: minuteOfDay, end: minuteOfDay, upper, lower };
      rows.push(current);
    }
  }
  return rows;
}

function meridiemPmTokens(ld) {
  const set = new Set();
  for (let minuteOfDay = 12 * 60; minuteOfDay < 24 * 60; ++minuteOfDay) {
    const hour = Math.floor(minuteOfDay / 60);
    const minute = minuteOfDay % 60;
    set.add(ld.meridiem(hour, minute, false));
    set.add(ld.meridiem(hour, minute, true));
  }
  return [...set].filter(Boolean).sort();
}

function transformMap(ld, fnName) {
  const fn = ld[fnName];
  if (typeof fn !== 'function') return [];
  const probes = [
    ...'0123456789',
    ',', '.', ':', '/', '-', '+',
    'A', 'P', 'M', 'a', 'p', 'm',
    '٠', '١', '٢', '٣', '٤', '٥', '٦', '٧', '٨', '٩',
    '۰', '۱', '۲', '۳', '۴', '۵', '۶', '۷', '۸', '۹',
    '०', '१', '२', '३', '४', '५', '६', '७', '८', '९',
    '০', '১', '২', '৩', '৪', '৫', '৬', '৭', '৮', '৯',
  ];
  const rows = [];
  for (const ch of [...new Set(probes)]) {
    const out = fn(ch);
    if (out !== ch) rows.push([ch, out]);
  }
  return rows;
}

function normalizeForParse(ld, value) {
  const text = String(value ?? '');
  if (!text) return '';
  return typeof ld.preparse === 'function' ? ld.preparse(text) : text;
}

function addTextVariant(values, value) {
  const text = String(value ?? '');
  if (!text) return;

  const variants = new Set([text]);
  variants.add(text.toLocaleLowerCase());
  variants.add(text.toLocaleUpperCase());

  if (text.endsWith('.')) {
    variants.add(text.slice(0, -1));
    variants.add(text.slice(0, -1).toLocaleLowerCase());
    variants.add(text.slice(0, -1).toLocaleUpperCase());
  } else {
    variants.add(`${text}.`);
  }

  for (const variant of variants) {
    if (variant) values.add(variant);
  }
}

function addRawArrayValue(values, raw, index) {
  if (Array.isArray(raw)) {
    addTextVariant(values, raw[index]);
  } else if (raw && typeof raw === 'object') {
    addRawArrayValue(values, raw.format, index);
    addRawArrayValue(values, raw.standalone, index);
  }
}

function parseLocaleValue(ld, method, text, token, strict) {
  if (typeof ld[method] !== 'function') return null;
  try {
    const parsed = ld[method](text, token, strict);
    return Number.isInteger(parsed) ? parsed : null;
  } catch {
    return null;
  }
}

function sortedParseEntries(entriesByText) {
  return [...entriesByText.values()].sort((a, b) => {
    if (b.text.length !== a.text.length) return b.text.length - a.text.length;
    return a.text.localeCompare(b.text);
  });
}

function buildParseEntries(ld, method, token, count, candidateBuilder) {
  const candidates = new Set();
  for (let index = 0; index < count; ++index) {
    for (const candidate of candidateBuilder(index)) {
      addTextVariant(candidates, candidate);
    }
  }

  const entriesByText = new Map();
  for (const candidate of candidates) {
    const text = normalizeForParse(ld, candidate);
    if (!text) continue;

    const strictValue = parseLocaleValue(ld, method, text, token, true);
    const lenientValue = parseLocaleValue(ld, method, text, token, false);
    const value = strictValue ?? lenientValue;
    if (!Number.isInteger(value) || value < 0 || value >= count) {
      continue;
    }

    const existing = entriesByText.get(text);
    if (existing) {
      existing.strict = existing.strict || strictValue === value;
    } else {
      entriesByText.set(text, {
        text,
        value,
        strict: strictValue === value,
      });
    }
  }

  return sortedParseEntries(entriesByText);
}

function monthCandidates(ld, month) {
  const m = momentForMonth(month);
  const values = new Set();
  for (const format of ['', 'MMMM', 'MMM', 'D MMMM', 'D MMM', 'LL', 'LLL', 'LLLL']) {
    addTextVariant(values, ld.months(m, format));
    addTextVariant(values, ld.monthsShort(m, format));
  }
  addRawArrayValue(values, ld._months, month);
  addRawArrayValue(values, ld._monthsShort, month);
  return values;
}

function weekdayCandidates(ld, day) {
  const m = momentForWeekday(day).value;
  const values = new Set();
  for (const format of ['', 'dddd', 'ddd', 'dd', '[on] dddd', '[at] dddd', 'dddd, MMMM D', 'LLLL']) {
    addTextVariant(values, ld.weekdays(m, format));
    addTextVariant(values, ld.weekdaysShort(m, format));
    addTextVariant(values, ld.weekdaysMin(m, format));
  }
  addRawArrayValue(values, ld._weekdays, day);
  addRawArrayValue(values, ld._weekdaysShort, day);
  addRawArrayValue(values, ld._weekdaysMin, day);
  return values;
}

function regexFullyAccepts(regex, text) {
  if (!(regex instanceof RegExp)) return true;
  const flags = regex.flags.replace('g', '').replace('y', '');
  return new RegExp(`^(?:${regex.source})$`, flags).test(text);
}

function meridiemParseEntries(ld) {
  const candidates = new Set();
  for (let minuteOfDay = 0; minuteOfDay < 24 * 60; ++minuteOfDay) {
    const hour = Math.floor(minuteOfDay / 60);
    const minute = minuteOfDay % 60;
    addTextVariant(candidates, ld.meridiem(hour, minute, false));
    addTextVariant(candidates, ld.meridiem(hour, minute, true));
  }

  const entriesByText = new Map();
  for (const candidate of candidates) {
    const text = normalizeForParse(ld, candidate);
    if (!text || !regexFullyAccepts(ld._meridiemParse, text)) continue;
    entriesByText.set(text, {
      text,
      value: ld.isPM(text) ? 1 : 0,
      strict: true,
    });
  }

  return sortedParseEntries(entriesByText);
}

function emitMeridiemHour(ld, entries) {
  if (typeof ld.meridiemHour !== 'function' || !entries.length) {
    return null;
  }

  const rows = entries.map((entry) => {
    const hours = numberRange(0, 12).map((hour) => {
      const value = ld.meridiemHour(hour, entry.text);
      return Number.isInteger(value) ? value : hour;
    });
    return { text: entry.text, hours };
  });

  return `[](int hour, const std::string& meridiem) -> int {\n`
    + `        struct Row { const char* text; std::array<int, 13> hours; };\n`
    + `        static const Row rows[] = {\n`
    + rows.map((row) => `            {${cppString(row.text)}, {{${row.hours.join(', ')}}}}`).join(',\n')
    + `\n        };\n`
    + `        const int index = std::clamp(hour, 0, 12);\n`
    + `        for (const auto& row : rows) {\n`
    + `            if (meridiem == row.text) return row.hours[index];\n`
    + `        }\n`
    + `        return hour;\n`
    + `    }`;
}

function ordinalParseEntries(ld, token, start, end) {
  const regex = ld._dayOfMonthOrdinalParse || ld._ordinalParse;
  const entriesByText = new Map();

  for (const value of numberRange(start, end)) {
    const candidates = new Set();
    addTextVariant(candidates, ld.ordinal(value, token));
    for (const candidate of candidates) {
      const text = normalizeForParse(ld, candidate);
      if (!text || !regexFullyAccepts(regex, text)) continue;
      entriesByText.set(text, { text, value, strict: true });
    }
  }

  return sortedParseEntries(entriesByText);
}

function emitParseEntryVector(entries) {
  if (!entries.length) return '{}';
  return `{\n${entries.map((entry) => (
    `        LocaleParseEntry{${cppString(entry.text)}, ${entry.value}, ${entry.strict ? 'true' : 'false'}}`
  )).join(',\n')}\n    }`;
}

function eraTimestampExpr(value, defaultDirection = 1) {
  if (value === undefined) {
    return defaultDirection < 0
      ? 'std::numeric_limits<int64_t>::min()'
      : 'std::numeric_limits<int64_t>::max()';
  }
  if (value === Infinity) return 'std::numeric_limits<int64_t>::max()';
  if (value === -Infinity) return 'std::numeric_limits<int64_t>::min()';
  if (typeof value === 'number') return `${Math.trunc(value)}LL`;

  const parsed = moment.utc(String(value)).startOf('day');
  if (!parsed.isValid()) {
    throw new Error(`unable to parse era boundary ${value}`);
  }
  return `${Math.trunc(parsed.valueOf())}LL`;
}

function emitEraYearOrdinalParser(ld) {
  const one = String(ld.ordinal(1, 'y'));
  const two = String(ld.ordinal(2, 'y'));
  const specialOne = one.includes('1') ? null : one;
  const twoIndex = two.indexOf('2');
  const prefix = twoIndex >= 0 ? two.slice(0, twoIndex) : '';
  const suffix = twoIndex >= 0 ? two.slice(twoIndex + 1) : '';
  const lines = [];

  lines.push('[](const std::string& input, size_t& parsedLength) -> int {');
  lines.push('        parsedLength = 0;');
  if (specialOne) {
    lines.push(`        const std::string specialOne = ${cppString(specialOne)};`);
    lines.push('        if (input.rfind(specialOne, 0) == 0) {');
    lines.push('            parsedLength = specialOne.size();');
    lines.push('            return 1;');
    lines.push('        }');
  }
  if (prefix) {
    lines.push(`        const std::string prefix = ${cppString(prefix)};`);
    lines.push('        if (input.rfind(prefix, 0) != 0) return 0;');
    lines.push('        size_t pos = prefix.size();');
  } else {
    lines.push('        size_t pos = 0;');
  }
  lines.push('        size_t digitsStart = pos;');
  lines.push('        int value = 0;');
  lines.push('        while (pos < input.size() && input[pos] >= \'0\' && input[pos] <= \'9\') {');
  lines.push('            value = value * 10 + (input[pos] - \'0\');');
  lines.push('            ++pos;');
  lines.push('        }');
  lines.push('        if (pos == digitsStart) return 0;');
  if (suffix) {
    lines.push(`        const std::string suffix = ${cppString(suffix)};`);
    lines.push('        if (input.compare(pos, suffix.size(), suffix) != 0) return 0;');
    lines.push('        pos += suffix.size();');
  }
  lines.push('        parsedLength = pos;');
  lines.push('        return value;');
  lines.push('    }');
  return lines.join('\n');
}

function emitEras(ld) {
  const eras = Array.isArray(ld._eras) ? ld._eras : [];
  if (!eras.length) return [];

  const lines = ['    loc.eras = {'];
  for (const era of eras) {
    const sinceExpr = eraTimestampExpr(era.since, 1);
    const untilExpr = eraTimestampExpr(era.until, era.until === -Infinity ? -1 : 1);
    lines.push('        EraSpec{');
    lines.push(`            ${sinceExpr},`);
    lines.push(`            ${untilExpr},`);
    lines.push(`            ${era.offset ?? 1},`);
    lines.push(`            ${cppString(era.name)},`);
    lines.push(`            ${cppString(era.narrow)},`);
    lines.push(`            ${cppString(era.abbr)}`);
    lines.push('        },');
  }
  lines.push('    };');

  if (typeof ld.eraYearOrdinalParse === 'function') {
    lines.push(`    loc.eraYearOrdinalParse = ${emitEraYearOrdinalParser(ld)};`);
  }

  return lines;
}

function emitTransform(rows) {
  return `[](const std::string& input) -> std::string {\n`
    + `        static const std::pair<const char*, const char*> replacements[] = {\n`
    + rows.map(([from, to]) => `            {${cppString(from)}, ${cppString(to)}}`).join(',\n')
    + `\n        };\n`
    + `        return generated::replaceAll(input, replacements);\n`
    + `    }`;
}

function emitLocale(code) {
  const id = ident(code);
  const ld = moment.localeData(code);
  const rt = ld._relativeTime || {};
  const calendar = ld._calendar || {};
  const calendarKeys = ['sameDay', 'nextDay', 'nextWeek', 'lastDay', 'lastWeek', 'sameElse'];
  const relKeys = ['s', 'ss', 'm', 'mm', 'h', 'hh', 'd', 'dd', 'w', 'ww', 'M', 'MM', 'y', 'yy'];
  const wrapperFunctions = typeof rt.future === 'function' || typeof rt.past === 'function';

  const months = numberRange(0, 11).map((month) => ld.months(momentForMonth(month), 'D MMMM'));
  const monthsShort = numberRange(0, 11).map((month) => ld.monthsShort(momentForMonth(month), 'D MMM'));
  const monthsStandalone = numberRange(0, 11).map((month) => ld.months(momentForMonth(month), ''));
  const monthsShortStandalone = numberRange(0, 11).map((month) => ld.monthsShort(momentForMonth(month), ''));
  const weekdaysFormatRegexSource =
    ld._weekdays && !Array.isArray(ld._weekdays) && ld._weekdays.isFormat
      ? ld._weekdays.isFormat.source
      : '';
  const weekdays = numberRange(0, 6).map((day) => ld.weekdays(momentForWeekday(day).value, 'dddd'));
  const weekdaysStandalone = numberRange(0, 6).map((day) => ld.weekdays(momentForWeekday(day).value, ''));
  const weekdaysFormat = numberRange(0, 6).map((day) =>
    ld.weekdays(momentForWeekday(day).value, weekdayFormatSample(weekdaysFormatRegexSource)));
  const weekdaysShort = numberRange(0, 6).map((day) => ld.weekdaysShort(momentForWeekday(day).value));
  const weekdaysMin = numberRange(0, 6).map((day) => ld.weekdaysMin(momentForWeekday(day).value));
  const monthsParse = buildParseEntries(ld, 'monthsParse', 'MMMM', 12, (month) => monthCandidates(ld, month));
  const monthsShortParse = buildParseEntries(ld, 'monthsParse', 'MMM', 12, (month) => monthCandidates(ld, month));
  const weekdaysParse = buildParseEntries(ld, 'weekdaysParse', 'dddd', 7, (day) => weekdayCandidates(ld, day));
  const weekdaysShortParse = buildParseEntries(ld, 'weekdaysParse', 'ddd', 7, (day) => weekdayCandidates(ld, day));
  const weekdaysMinParse = buildParseEntries(ld, 'weekdaysParse', 'dd', 7, (day) => weekdayCandidates(ld, day));
  const meridiemParse = meridiemParseEntries(ld);
  const dayOfMonthOrdinalParse = ordinalParseEntries(ld, 'D', 1, 31);
  const monthOrdinalParse = ordinalParseEntries(ld, 'M', 1, 12);

  const lines = [];
  lines.push(`inline LocaleData buildLocale_${id}() {`);
  lines.push('    LocaleData loc;');
  lines.push(`    loc.name = ${cppString(code)};`);
  lines.push(`    loc.months = ${arrayInit(months)};`);
  lines.push(`    loc.monthsShort = ${arrayInit(monthsShort)};`);
  lines.push(`    loc.monthsStandalone = ${arrayInit(monthsStandalone)};`);
  lines.push(`    loc.monthsShortStandalone = ${arrayInit(monthsShortStandalone)};`);
  lines.push(`    loc.weekdays = ${arrayInit(weekdays)};`);
  lines.push(`    loc.weekdaysShort = ${arrayInit(weekdaysShort)};`);
  lines.push(`    loc.weekdaysMin = ${arrayInit(weekdaysMin)};`);
  lines.push(`    loc.weekdaysStandalone = ${arrayInit(weekdaysStandalone)};`);
  lines.push(`    loc.weekdaysFormat = ${arrayInit(weekdaysFormat)};`);
  if (weekdaysFormatRegexSource) {
    lines.push(`    loc.weekdaysFormatRegexSource = ${cppString(weekdaysFormatRegexSource)};`);
  }
  lines.push(`    loc.monthsParse = ${emitParseEntryVector(monthsParse)};`);
  lines.push(`    loc.monthsShortParse = ${emitParseEntryVector(monthsShortParse)};`);
  lines.push(`    loc.weekdaysParse = ${emitParseEntryVector(weekdaysParse)};`);
  lines.push(`    loc.weekdaysShortParse = ${emitParseEntryVector(weekdaysShortParse)};`);
  lines.push(`    loc.weekdaysMinParse = ${emitParseEntryVector(weekdaysMinParse)};`);
  lines.push(`    loc.meridiemParse = ${emitParseEntryVector(meridiemParse)};`);
  lines.push(`    loc.dayOfMonthOrdinalParse = ${emitParseEntryVector(dayOfMonthOrdinalParse)};`);
  lines.push(`    loc.monthOrdinalParse = ${emitParseEntryVector(monthOrdinalParse)};`);
  lines.push(`    loc.longDateFormat.LT = ${cppString(ld.longDateFormat('LT'))};`);
  lines.push(`    loc.longDateFormat.LTS = ${cppString(ld.longDateFormat('LTS'))};`);
  lines.push(`    loc.longDateFormat.L = ${cppString(ld.longDateFormat('L'))};`);
  lines.push(`    loc.longDateFormat.LL = ${cppString(ld.longDateFormat('LL'))};`);
  lines.push(`    loc.longDateFormat.LLL = ${cppString(ld.longDateFormat('LLL'))};`);
  lines.push(`    loc.longDateFormat.LLLL = ${cppString(ld.longDateFormat('LLLL'))};`);
  lines.push(`    loc.week.dow = ${ld.firstDayOfWeek()};`);
  lines.push(`    loc.week.doy = ${ld.firstDayOfYear()};`);

  for (const key of calendarKeys) {
    const value = calendar[key];
    if (typeof value === 'function') {
      lines.push(`    loc.calendar.${key} = CalendarFn([](const Moment& moment, const Moment& reference) -> std::string {`);
      lines.push(`        static const std::array<std::array<std::array<const char*, 7>, 24>, 2> values = ${emitCalendarTable(calendarTable(ld, key))};`);
      lines.push('        int sameWeek = moment.week() == reference.week() ? 0 : 1;');
      lines.push('        int hour = std::clamp(moment.hour(), 0, 23);');
      lines.push('        int day = std::clamp(moment.day(), 0, 6);');
      lines.push('        return values[sameWeek][hour][day];');
      lines.push('    });');
    } else {
      lines.push(`    loc.calendar.${key} = std::string(${cppString(ld.calendar(key, momentForWeekday(0).value, momentForWeekday(0).reference))});`);
    }
  }

  if (wrapperFunctions) {
    lines.push('    loc.relativeTime.future = "%s";');
    lines.push('    loc.relativeTime.past = "%s";');
  } else {
    lines.push(`    loc.relativeTime.future = ${cppString(String(rt.future || 'in %s'))};`);
    lines.push(`    loc.relativeTime.past = ${cppString(String(rt.past || '%s ago'))};`);
  }

  for (const key of relKeys) {
    const rel = relativeOutputs(ld, rt, key, wrapperFunctions);
    if (rel.kind === 'string') {
      lines.push(`    loc.relativeTime.${key} = std::string(${cppString(rel.value)});`);
    } else {
      lines.push(`    loc.relativeTime.${key} = RelativeTimeFn([](int number, bool withoutSuffix, const std::string&, bool isFuture) -> std::string {`);
      lines.push(emitRelativeTable(rel));
      lines.push('        const auto& table = withoutSuffix');
      lines.push('            ? (isFuture ? noSuffixFuture : noSuffixPast)');
      lines.push('            : (isFuture ? withSuffixFuture : withSuffixPast);');
      lines.push('        int index = std::clamp(number, 0, 200);');
      lines.push('        std::string result = table[index];');
      lines.push('        if (number > 200) generated::replaceFirst(result, "200", std::to_string(number));');
      lines.push('        return result;');
      lines.push('    });');
    }
  }

  const ordinalSpecs = [
    ['y', 1, 3000],
    ['Q', 1, 4],
    ['M', 1, 12],
    ['D', 1, 31],
    ['d', 0, 6],
    ['w', 1, 53],
    ['W', 1, 53],
  ];
  lines.push('    loc.ordinal = [](int number, const std::string& token) -> std::string {');
  for (const [token, start, end] of ordinalSpecs) {
    lines.push(`        if (token == ${cppString(token)} && number >= ${start} && number <= ${end}) {`);
    lines.push(`            static const std::array<const char*, ${end - start + 1}> values = ${arrayInit(ordinalTable(ld, token, numberRange(start, end)), '                ')};`);
    lines.push(`            return values[number - ${start}];`);
    lines.push('        }');
  }
  lines.push('        return std::to_string(number);');
  lines.push('    };');

  const segments = meridiemSegments(ld);
  const defaultMeridiem = segments.length === 2
    && segments[0].start === 0 && segments[0].end === 719
    && segments[0].upper === 'AM' && segments[0].lower === 'am'
    && segments[1].start === 720 && segments[1].end === 1439
    && segments[1].upper === 'PM' && segments[1].lower === 'pm';
  if (!defaultMeridiem) {
    lines.push('    loc.meridiem = [](int hour, int minute, bool isLower) -> std::string {');
    lines.push('        static const generated::MeridiemSegment segments[] = {');
    lines.push(segments.map((seg) => `            {${seg.start}, ${seg.end}, ${cppString(seg.upper)}, ${cppString(seg.lower)}}`).join(',\n'));
    lines.push('        };');
    lines.push('        return generated::lookupMeridiem(segments, hour, minute, isLower);');
    lines.push('    };');
  } else {
    lines.push('    loc.meridiem = [](int hour, int, bool isLower) -> std::string { return hour >= 12 ? (isLower ? "pm" : "PM") : (isLower ? "am" : "AM"); };');
  }

  const pmTokens = meridiemPmTokens(ld);
  lines.push('    loc.isPM = [](const std::string& input) -> bool {');
  lines.push('        static const char* values[] = {');
  lines.push(pmTokens.map((value) => `            ${cppString(value)}`).join(',\n'));
  lines.push('        };');
  lines.push('        return generated::contains(values, input);');
  lines.push('    };');

  const meridiemHour = emitMeridiemHour(ld, meridiemParse);
  if (meridiemHour) {
    lines.push(`    loc.meridiemHour = ${meridiemHour};`);
  }

  const preparse = transformMap(ld, 'preparse');
  const postformat = transformMap(ld, 'postformat');
  lines.push(`    loc.preparse = ${preparse.length ? emitTransform(preparse) : '[](const std::string& input) -> std::string { return input; }'};`);
  lines.push(`    loc.postformat = ${postformat.length ? emitTransform(postformat) : '[](const std::string& input) -> std::string { return input; }'};`);
  lines.push(...emitEras(ld));
  lines.push(`    loc.invalidDate = ${cppString(ld.invalidDate())};`);
  lines.push('    return loc;');
  lines.push('}');
  return lines.join('\n');
}

const header = [];
header.push('// Generated by tools/generate-locales.mjs. Do not edit by hand.');
header.push(`// Source: Moment.js ${moment.version} locale files from the npm package.`);
header.push('#pragma once');
header.push('');
header.push('#include <polycpp/moment/locale.hpp>');
header.push('');
header.push('#include <algorithm>');
header.push('#include <array>');
header.push('#include <cstdint>');
header.push('#include <cstring>');
header.push('#include <limits>');
header.push('#include <string>');
header.push('#include <utility>');
header.push('');
header.push('namespace polycpp {');
header.push('namespace moment {');
header.push('namespace detail {');
header.push('namespace generated {');
header.push('');
header.push('inline void replaceFirst(std::string& input, const std::string& from, const std::string& to) {');
header.push('    auto pos = input.find(from);');
header.push('    if (pos != std::string::npos) input.replace(pos, from.size(), to);');
header.push('}');
header.push('');
header.push('template <size_t N>');
header.push('inline std::string replaceAll(std::string input, const std::pair<const char*, const char*> (&replacements)[N]) {');
header.push('    for (const auto& [from, to] : replacements) {');
header.push('        size_t pos = 0;');
header.push('        const std::string needle(from);');
header.push('        const std::string replacement(to);');
header.push('        while ((pos = input.find(needle, pos)) != std::string::npos) {');
header.push('            input.replace(pos, needle.size(), replacement);');
header.push('            pos += replacement.size();');
header.push('        }');
header.push('    }');
header.push('    return input;');
header.push('}');
header.push('');
header.push('struct MeridiemSegment {');
header.push('    int start;');
header.push('    int end;');
header.push('    const char* upper;');
header.push('    const char* lower;');
header.push('};');
header.push('');
header.push('template <size_t N>');
header.push('inline std::string lookupMeridiem(const MeridiemSegment (&segments)[N], int hour, int minute, bool isLower) {');
header.push('    int minuteOfDay = std::clamp(hour, 0, 23) * 60 + std::clamp(minute, 0, 59);');
header.push('    for (const auto& segment : segments) {');
header.push('        if (minuteOfDay >= segment.start && minuteOfDay <= segment.end) {');
header.push('            return isLower ? segment.lower : segment.upper;');
header.push('        }');
header.push('    }');
header.push('    return hour >= 12 ? (isLower ? "pm" : "PM") : (isLower ? "am" : "AM");');
header.push('}');
header.push('');
header.push('template <size_t N>');
header.push('inline bool contains(const char* const (&values)[N], const std::string& input) {');
header.push('    for (const char* value : values) {');
header.push('        if (input == value) return true;');
header.push('    }');
header.push('    return false;');
header.push('}');
header.push('');
header.push('} // namespace generated');
header.push('');
for (const file of localeFiles) {
  const code = file.slice(0, -3);
  header.push(`#if defined(${macroName(code)})`);
  header.push(emitLocale(code));
  header.push('#endif');
  header.push('');
}
header.push('inline void registerGeneratedMomentLocales() {');
for (const file of localeFiles) {
  const code = file.slice(0, -3);
  header.push(`#if defined(${macroName(code)})`);
  header.push(`    defineLocale(${cppString(code)}, buildLocale_${ident(code)}());`);
  header.push('#endif');
}
header.push('}');
header.push('');
header.push('} // namespace detail');
header.push('} // namespace moment');
header.push('} // namespace polycpp');
header.push('');

fs.mkdirSync(path.dirname(outPath), { recursive: true });
fs.writeFileSync(outPath, header.join('\n'));

const localeCodes = localeFiles.map((file) => file.slice(0, -3));
const cmake = [];
cmake.push('# Generated by tools/generate-locales.mjs. Do not edit by hand.');
cmake.push('');
cmake.push('set(POLYCPP_MOMENT_AVAILABLE_LOCALES');
for (const code of localeCodes) {
  cmake.push(`    "${code}"`);
}
cmake.push(')');
cmake.push('');
cmake.push('option(POLYCPP_MOMENT_ENABLE_ALL_LOCALES "Compile and register all generated Moment.js locales when POLYCPP_MOMENT_LOCALES is empty" ON)');
cmake.push('set(POLYCPP_MOMENT_LOCALES "" CACHE STRING "Comma or semicolon separated Moment.js locale keys to compile/register; overrides POLYCPP_MOMENT_ENABLE_ALL_LOCALES when non-empty")');
cmake.push('set_property(CACHE POLYCPP_MOMENT_LOCALES PROPERTY STRINGS ${POLYCPP_MOMENT_AVAILABLE_LOCALES})');
cmake.push('');
cmake.push('function(polycpp_moment_configure_locales target)');
cmake.push('    set(_requested "${POLYCPP_MOMENT_LOCALES}")');
cmake.push('    string(REPLACE "," ";" _requested "${_requested}")');
cmake.push('    string(REPLACE " " ";" _requested "${_requested}")');
cmake.push('');
cmake.push('    set(_selected)');
cmake.push('    if(_requested)');
cmake.push('        foreach(_locale IN LISTS _requested)');
cmake.push('            string(STRIP "${_locale}" _locale)');
cmake.push('            if(_locale STREQUAL "")');
cmake.push('                continue()');
cmake.push('            endif()');
cmake.push('            string(TOLOWER "${_locale}" _locale)');
cmake.push('            if(_locale STREQUAL "all")');
cmake.push('                list(APPEND _selected ${POLYCPP_MOMENT_AVAILABLE_LOCALES})');
cmake.push('            elseif(_locale STREQUAL "none")');
cmake.push('                set(_selected)');
cmake.push('            else()');
cmake.push('                list(FIND POLYCPP_MOMENT_AVAILABLE_LOCALES "${_locale}" _locale_index)');
cmake.push('                if(_locale_index EQUAL -1)');
cmake.push('                    message(FATAL_ERROR "Unknown Moment.js locale \'${_locale}\'. Valid locales: ${POLYCPP_MOMENT_AVAILABLE_LOCALES}")');
cmake.push('                endif()');
cmake.push('                list(APPEND _selected "${_locale}")');
cmake.push('            endif()');
cmake.push('        endforeach()');
cmake.push('    elseif(POLYCPP_MOMENT_ENABLE_ALL_LOCALES)');
cmake.push('        set(_selected ${POLYCPP_MOMENT_AVAILABLE_LOCALES})');
cmake.push('    endif()');
cmake.push('');
cmake.push('    if(_selected)');
cmake.push('        list(REMOVE_DUPLICATES _selected)');
cmake.push('    endif()');
cmake.push('');
cmake.push('    list(LENGTH POLYCPP_MOMENT_AVAILABLE_LOCALES _available_count)');
cmake.push('    list(LENGTH _selected _selected_count)');
cmake.push('    set(_all_enabled FALSE)');
cmake.push('    if(_selected_count EQUAL _available_count)');
cmake.push('        set(_all_enabled TRUE)');
cmake.push('    endif()');
cmake.push('');
cmake.push('    set(POLYCPP_MOMENT_SELECTED_LOCALES "${_selected}" PARENT_SCOPE)');
cmake.push('    set(POLYCPP_MOMENT_ALL_GENERATED_LOCALES_ENABLED "${_all_enabled}" PARENT_SCOPE)');
cmake.push('');
cmake.push('    if(_selected)');
cmake.push('        target_sources(${target} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../src/generated_locales.cpp")');
cmake.push('        foreach(_locale IN LISTS _selected)');
cmake.push('            string(MAKE_C_IDENTIFIER "${_locale}" _locale_ident)');
cmake.push('            target_compile_definitions(${target} PRIVATE "POLYCPP_MOMENT_LOCALE_${_locale_ident}")');
cmake.push('        endforeach()');
cmake.push('        message(STATUS "polycpp_moment generated locales: ${_selected_count} enabled")');
cmake.push('    else()');
cmake.push('        target_compile_definitions(${target} PUBLIC POLYCPP_MOMENT_DISABLE_GENERATED_LOCALES)');
cmake.push('        message(STATUS "polycpp_moment generated locales: disabled")');
cmake.push('    endif()');
cmake.push('endfunction()');
cmake.push('');

fs.mkdirSync(path.dirname(cmakeOutPath), { recursive: true });
fs.writeFileSync(cmakeOutPath, cmake.join('\n'));
console.log(`generated ${localeFiles.length} locales: ${path.relative(repoRoot, outPath)}, ${path.relative(repoRoot, cmakeOutPath)}`);
